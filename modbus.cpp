/*
 * C/C++ Library Includes
 */

#include <stdint.h>
#include <stdbool.h>

/*
 * Modbus Library Includes
 */

#include "modbus.h"

/*
 * Private Module Functions
 */

static uint16_t get_crc16(uint8_t * buffer, uint8_t number_of_bytes)
{
    uint16_t crc = 0xFFFF;
 
    for (int pos = 0; pos < number_of_bytes; pos++)
    {
        crc ^= (uint16_t)buffer[pos];
 
        for (uint8_t i = 8; i != 0; i--)
        {
            if ((crc & 0x0001) != 0)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
              crc >>= 1;
            }
        }

    }
    return crc;
}

static int get_number_of_required_bytes_for_number_of_bits(uint16_t n_bits)
{
  return (n_bits & 7) ? (n_bits / 8) + 1 : n_bits / 8;
}

static int modbus_write_read_discrete_inputs_response_data_bytes(uint8_t * buffer, bool * discrete_inputs, uint8_t n_inputs)
{
    int bool_count = 0;
    int count = 0;

    int required_bytes = get_number_of_required_bytes_for_number_of_bits(n_inputs);
    buffer[count++] = (uint8_t)required_bytes;

    for (int b = 0; b < required_bytes; b++)
    {
        buffer[count] = 0x00;

        for (int i = 0; i < 7; i++)
        {
            buffer[count] |= discrete_inputs[bool_count] ? (1 << i) : 0;
            bool_count++;
            if(bool_count == n_inputs) { break; }
        }

        count++;
    }

    return count;
}

static uint16_t bytes_to_uint16_t(uint8_t * bytes)
{
    return (bytes[0] << 8) + bytes[1];
}

static int16_t bytes_to_int16_t(uint8_t * bytes)
{
    return (bytes[0] << 8) + bytes[1];
}

static void copy_to_holding_registers(uint16_t n_registers, uint8_t * data, int16_t * holding_registers)
{
    uint16_t i;
    for (i = 0; i < n_registers; i++)
    {
        holding_registers[i] = bytes_to_int16_t(data + (i * 2));
    }   
}

static void copy_byte_to_coils(uint8_t data, uint16_t n_coils, bool * coils)
{
    uint8_t i = 1;
    uint8_t coil = 0;
    n_coils = (n_coils <= 8) ? n_coils : 8; 

    for(i = 1; coil < n_coils; i <<= 1)
    {
        coils[coil] = data & i;
        coil++;
    }
}

static void copy_to_multiple_coils(uint16_t n_coils, uint8_t *data, bool * coils)
{
    uint8_t byte = 0;
    while(n_coils >= 8)
    {
        copy_byte_to_coils(data[byte], n_coils, coils);
        n_coils -= 8;
        byte++;
    }
    copy_byte_to_coils(data[byte], n_coils, coils);

}

static bool bytes_to_on_off_data(uint8_t * bytes)
{
    return bytes[0] == 0xFF;
}

static bool is_valid_coil_address(uint16_t coil_addr, const MODBUS_HANDLER& handler)
{
    return ((coil_addr > 0) && (coil_addr <= handler.data.num_coils));
}

static bool is_valid_input_register_addr(uint16_t input_register_addr, const MODBUS_HANDLER& handler)
{
    return ((input_register_addr > 0) && (input_register_addr <= handler.data.num_input_registers));
}

static bool is_valid_holding_register_addr(uint16_t holding_register_addr, const MODBUS_HANDLER& handler)
{
    return ((holding_register_addr > 0) && (holding_register_addr <= handler.data.num_holding_registers));
}

static bool is_valid_discrete_input_addr(uint16_t discrete_input_addr, const MODBUS_HANDLER& handler)
{
    return ((discrete_input_addr > 0) && (discrete_input_addr <= handler.data.num_inputs));
}

static bool is_valid_function_code(uint8_t code)
{
    bool valid = false;
    valid |= (code == READ_COILS);
    valid |= (code == READ_DISCRETE_INPUTS);
    valid |= (code == WRITE_SINGLE_COIL);
    valid |= (code == WRITE_MULTIPLE_COILS);
    valid |= (code == READ_INPUT_REGISTERS);
    valid |= (code == READ_HOLDING_REGISTERS);
    valid |= (code == WRITE_HOLDING_REGISTER);
    valid |= (code == WRITE_HOLDING_REGISTERS);
    valid |= (code == READ_WRITE_REGISTERS);
    valid |= (code == MASK_WRITE_REGISTER);
    return valid;
}

static uint8_t get_message_address(char const * const message)
{
    return (uint8_t)message[0];
}

static MODBUS_FUNCTION_CODE get_message_function_code(char const * const message)
{
    return (MODBUS_FUNCTION_CODE)message[1];
}

static MODBUS_EXCEPTION_CODES handle_read_coils(void const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.read_coils) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t first_coil = bytes_to_uint16_t((uint8_t*)data);
    uint16_t n_coils = bytes_to_uint16_t((uint8_t*)data+2);
    uint16_t last_coil = first_coil + n_coils - 1;

    if (!is_valid_coil_address(first_coil, handler) || !is_valid_coil_address(last_coil, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    handler.functions.read_coils(first_coil, n_coils);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_read_discrete_inputs(void const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.read_discrete_inputs) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t first_input = bytes_to_uint16_t((uint8_t*)data);
    uint16_t n_inputs = bytes_to_uint16_t((uint8_t*)data+2);
    uint16_t last_input = first_input + n_inputs - 1;

    if (!is_valid_discrete_input_addr(first_input, handler) || !is_valid_discrete_input_addr(last_input, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    handler.functions.read_discrete_inputs(first_input, n_inputs);

    return EXCEPTION_NONE;
}

static bool on_off_data_is_valid(uint8_t * data)
{
    bool valid_on_off_data = false;
    valid_on_off_data |= (data[0] == 0xFF) && (data[1] == 0x00);
    valid_on_off_data |= (data[0] == 0x00) && (data[1] == 0x00);
    return valid_on_off_data;
}

static MODBUS_EXCEPTION_CODES handle_write_single_coil(void const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.write_single_coil) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    if (!on_off_data_is_valid((uint8_t*)data + 2)) { return EXCEPTION_ILLEGAL_DATA_VALUE; }

    uint16_t coil = bytes_to_uint16_t((uint8_t*)data);

    if (!is_valid_coil_address(coil, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;  
    }

    bool on = bytes_to_on_off_data((uint8_t*)data + 2);

    handler.functions.write_single_coil(coil, on);
    
    return EXCEPTION_NONE;

}

static MODBUS_EXCEPTION_CODES handle_write_multiple_coils(char const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.write_multiple_coils) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t first_coil = bytes_to_uint16_t((uint8_t*)data);
    uint16_t n_coils = bytes_to_uint16_t((uint8_t*)data + 2);
    uint16_t last_coil = first_coil + n_coils - 1;

    if (!is_valid_coil_address(first_coil, handler) || !is_valid_coil_address(last_coil, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    copy_to_multiple_coils(n_coils, (uint8_t*)data + 5, handler.data.write_multiple_coils);
    handler.functions.write_multiple_coils(first_coil, n_coils, handler.data.write_multiple_coils);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_read_input_registers(char const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.read_input_registers) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t first_reg = bytes_to_uint16_t((uint8_t*)data);
    uint16_t n_registers = bytes_to_uint16_t((uint8_t*)data+2);
    uint16_t last_reg = first_reg + n_registers - 1;
    
    if (!is_valid_input_register_addr(first_reg, handler) || !is_valid_input_register_addr(last_reg, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    handler.functions.read_input_registers(first_reg, n_registers);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_read_holding_registers(char const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.read_holding_registers) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t first_reg = bytes_to_uint16_t((uint8_t*)data);
    uint16_t n_registers = bytes_to_uint16_t((uint8_t*)data+2);
    uint16_t last_reg = first_reg + n_registers - 1;

    if (!is_valid_holding_register_addr(first_reg, handler) || !is_valid_holding_register_addr(last_reg, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    handler.functions.read_holding_registers(first_reg, n_registers);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_write_holding_register(char const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.write_holding_register) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t reg = bytes_to_uint16_t((uint8_t*)data);
    int16_t value = bytes_to_int16_t((uint8_t*)data+2);

    if (!is_valid_holding_register_addr(reg, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    handler.functions.write_holding_register(reg, value);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_write_holding_registers(char const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.write_holding_registers) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t first_reg = bytes_to_uint16_t((uint8_t*)data);
    uint16_t n_registers = bytes_to_int16_t((uint8_t*)data+2);
    uint16_t last_reg = first_reg + n_registers - 1;

    uint8_t n_values = ((uint8_t*)data)[4];
    
    if (n_values != (n_registers * 2)) { return EXCEPTION_ILLEGAL_DATA_ADDRESS; }

    if (!is_valid_holding_register_addr(first_reg, handler) || !is_valid_holding_register_addr(last_reg, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    copy_to_holding_registers(n_registers, (uint8_t*)data + 5, handler.data.write_holding_registers);

    handler.functions.write_holding_registers(first_reg, n_registers, handler.data.write_holding_registers);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_read_write_registers(char const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.read_write_registers) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t read_start_reg = bytes_to_uint16_t((uint8_t*)data);
    uint16_t n_read_count = bytes_to_uint16_t((uint8_t*)data+2);
    uint16_t read_end_reg = read_start_reg + n_read_count - 1;
    
    uint16_t write_start_reg = bytes_to_uint16_t((uint8_t*)data+4);
    uint16_t n_write_count = bytes_to_uint16_t((uint8_t*)data+6);
    uint16_t write_end_reg = write_start_reg + n_write_count - 1;
    uint16_t write_byte_count = (uint16_t)data[8];

    bool bad_addresses = false;
    bool bad_write_byte_count = false;

    bad_addresses |= !is_valid_holding_register_addr(read_start_reg, handler);
    bad_addresses |= !is_valid_holding_register_addr(read_end_reg, handler);
    bad_addresses |= !is_valid_holding_register_addr(write_start_reg, handler);
    bad_addresses |= !is_valid_holding_register_addr(write_end_reg, handler);
    
    bad_write_byte_count = write_byte_count != (n_write_count * 2);

    if (bad_addresses || bad_write_byte_count) { return EXCEPTION_ILLEGAL_DATA_ADDRESS; }

    copy_to_holding_registers(n_write_count, (uint8_t*)data + 9, handler.data.write_holding_registers);

    handler.functions.read_write_registers(read_start_reg, n_read_count, write_start_reg, n_write_count, handler.data.write_holding_registers);

    return EXCEPTION_NONE;
}


static MODBUS_EXCEPTION_CODES handle_mask_write_register(char const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.mask_write_register) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t reg = bytes_to_uint16_t((uint8_t*)data);
    uint16_t and_mask = bytes_to_uint16_t((uint8_t*)data + 2);
    uint16_t or_mask = bytes_to_uint16_t((uint8_t*)data + 4);

    if (!is_valid_holding_register_addr(reg, handler)) { return EXCEPTION_ILLEGAL_DATA_ADDRESS; }
    
    handler.functions.mask_write_register(reg, and_mask, or_mask);

    return EXCEPTION_NONE;
}

static bool validate_message_crc(const char * message, int message_length)
{
    bool valid_crc = true;
    uint16_t expected_crc = get_crc16((uint8_t *)message, message_length - 2);
    valid_crc &= message[message_length-2] == (char)(expected_crc >> 8);
    valid_crc &= message[message_length-1] == (char)(expected_crc & 0xFF);
    return valid_crc;
}

/*
 * Public Module Functions
 */

void modbus_service_message(char const * const message, const MODBUS_HANDLER& handler, int message_length, bool check_crc)
{       
    MODBUS_FUNCTION_CODE function_code;

    if (!message) { return; }

    if (get_message_address(message) != handler.data.device_address) { return; }

    if (!is_valid_function_code(message[1])) { return; }

    if (check_crc && !validate_message_crc(message, message_length)) { return; }

    function_code = get_message_function_code(message);

    char const * const data_start = &message[2];

    MODBUS_EXCEPTION_CODES exception = EXCEPTION_ILLEGAL_FUNCTION_CODE;

    switch(function_code)
    {
    case READ_COILS:
        exception = handle_read_coils(data_start, handler);
        break;
    case READ_DISCRETE_INPUTS:
        exception = handle_read_discrete_inputs(data_start, handler);
        break;
    case WRITE_SINGLE_COIL:
        exception = handle_write_single_coil(data_start, handler);
        break;
    case WRITE_MULTIPLE_COILS:
        exception = handle_write_multiple_coils(data_start, handler);
        break;
    case READ_INPUT_REGISTERS:
        exception = handle_read_input_registers(data_start, handler);
        break;
    case READ_HOLDING_REGISTERS:
        exception = handle_read_holding_registers(data_start, handler);
        break;
    case WRITE_HOLDING_REGISTER:
        exception = handle_write_holding_register(data_start, handler);
        break;
    case WRITE_HOLDING_REGISTERS:
        exception = handle_write_holding_registers(data_start, handler);
        break;
    case READ_WRITE_REGISTERS:
        exception = handle_read_write_registers(data_start, handler);
        break;
    case MASK_WRITE_REGISTER:
        exception = handle_mask_write_register(data_start, handler);
        break;
    }

    if ((exception != EXCEPTION_NONE) && (handler.functions.exception_handler))
    {
        handler.functions.exception_handler(function_code + 128, exception);    
    }

}

int modbus_start_response(uint8_t * const buffer, MODBUS_FUNCTION_CODE function_code, uint8_t device_address)
{
    if (!buffer) { return 0;}

    buffer[0] = device_address;
    buffer[1] = (uint8_t)function_code;

    return 2;
}

int modbus_write(uint8_t * const buffer, int8_t value)
{
    if (!buffer) { return 0;}

    buffer[0] = value;

    return 1;
}

int modbus_write(uint8_t * const buffer, int16_t value)
{
    if (!buffer) { return 0;}

    buffer[0] = (value & 0xFF00) >> 8;
    buffer[1] = (value & 0x00FF);

    return 2;
}

int modbus_write_crc(uint8_t * const buffer, uint8_t bytes)
{
    uint16_t crc = get_crc16(buffer, bytes);
    uint8_t * crc_bytes = (uint8_t *)&crc;
    buffer[bytes] = crc_bytes[1];
    buffer[bytes+1] = crc_bytes[0];
    return 2;
}

int modbus_write_read_discrete_inputs_response(uint8_t source_address, uint8_t * buffer, bool * discrete_inputs, uint8_t n_inputs, bool add_crc)
{
    int count = 0;
    count += modbus_start_response(&buffer[count], READ_DISCRETE_INPUTS, source_address);
    count += modbus_write_read_discrete_inputs_response_data_bytes(&buffer[count], discrete_inputs, n_inputs);
    
    if (add_crc)
    {
        count += modbus_write_crc(buffer, count);
    }
    
    return count;
}

int modbus_write_read_input_registers_response(uint8_t source_address, uint8_t * buffer, int16_t * input_registers, uint8_t n_registers, bool add_crc)
{
    int count = 0;
    count += modbus_start_response(&buffer[count], READ_INPUT_REGISTERS, source_address);
    count += modbus_write(&buffer[count], (int8_t)n_registers);
    
    for (int i = 0; i < n_registers; i++)
    {
        count += modbus_write(&buffer[count], (int16_t)input_registers[i]);
    }

    if (add_crc)
    {
        count += modbus_write_crc(buffer, count);
    }

    return count;
}
