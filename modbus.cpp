/*
 * C/C++ Library Includes
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

//#include <iostream>

/*
 * Modbus Library Includes
 */

#include "modbus.h"

/*
 * Private Module Data
 */

static uint8_t const * s_current_message = NULL;
static int s_current_message_length = 0;
static bool s_broadcast;

/*
 * Private Module Functions
 */

CRC_CHECK_STATE application_check_crc(const uint8_t * message, int message_length, bool reverse_order = false);

#ifndef ALLOW_APPLICATION_CRC_CHECKS

#define application_check_crc(message, message_length, reverse_order) (CRC_NOT_CHECKED)

#endif

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

static uint16_t bytes_to_uint16_t(uint8_t const * const bytes)
{
    return (bytes[0] << 8) + bytes[1];
}

static int16_t bytes_to_int16_t(uint8_t const * const bytes)
{
    return (bytes[0] << 8) + bytes[1];
}

static void copy_to_holding_registers(uint16_t n_registers, uint8_t const * const data, int16_t * holding_registers)
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

static void copy_to_multiple_coils(uint16_t n_coils, uint8_t const * const data, bool * coils)
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

static bool bytes_to_on_off_data(uint8_t const * const bytes)
{
    return bytes[0] == 0xFF;
}

static bool is_valid_coil_address(uint16_t coil_addr, const MODBUS_HANDLER& handler)
{
    return (coil_addr < handler.data.num_coils);
}

static bool is_valid_input_register_addr(uint16_t input_register_addr, const MODBUS_HANDLER& handler)
{
    return (input_register_addr < handler.data.num_input_registers);
}

static bool is_valid_holding_register_addr(uint16_t holding_register_addr, const MODBUS_HANDLER& handler)
{
    return (holding_register_addr < handler.data.num_holding_registers);
}

static bool is_valid_discrete_input_addr(uint16_t discrete_input_addr, const MODBUS_HANDLER& handler)
{
    return (discrete_input_addr < handler.data.num_inputs);
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

static uint8_t get_message_address(uint8_t const * const message)
{
    return (uint8_t)message[0];
}

static MODBUS_FUNCTION_CODE get_message_function_code(uint8_t const * const message)
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

static bool on_off_data_is_valid(uint8_t const * const data)
{
    bool valid_on_off_data = false;
    valid_on_off_data |= (data[0] == 0xFF) && (data[1] == 0x00);
    valid_on_off_data |= (data[0] == 0x00) && (data[1] == 0x00);
    return valid_on_off_data;
}

static MODBUS_EXCEPTION_CODES handle_write_single_coil(uint8_t const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.write_single_coil) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    if (!on_off_data_is_valid(data + 2)) { return EXCEPTION_ILLEGAL_DATA_VALUE; }

    uint16_t coil = bytes_to_uint16_t(data);

    if (!is_valid_coil_address(coil, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;  
    }

    bool on = bytes_to_on_off_data(data + 2);

    handler.functions.write_single_coil(coil, on);
    
    return EXCEPTION_NONE;

}

static MODBUS_EXCEPTION_CODES handle_write_multiple_coils(uint8_t const * const data, const MODBUS_HANDLER& handler)
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

static MODBUS_EXCEPTION_CODES handle_read_input_registers(uint8_t const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.read_input_registers) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t first_reg = bytes_to_uint16_t(data);
    uint16_t n_registers = bytes_to_uint16_t(data+2);
    uint16_t last_reg = first_reg + n_registers - 1;
    
    if (!is_valid_input_register_addr(first_reg, handler) || !is_valid_input_register_addr(last_reg, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    handler.functions.read_input_registers(first_reg, n_registers);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_read_holding_registers(uint8_t const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.read_holding_registers) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t first_reg = bytes_to_uint16_t(data);
    uint16_t n_registers = bytes_to_uint16_t(data+2);
    uint16_t last_reg = first_reg + n_registers - 1;

    if (!is_valid_holding_register_addr(first_reg, handler) || !is_valid_holding_register_addr(last_reg, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }
    
    handler.functions.read_holding_registers(first_reg, n_registers);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_write_holding_register(uint8_t const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.write_holding_register) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t reg = bytes_to_uint16_t(data);
    int16_t value = bytes_to_int16_t(data+2);

    if (!is_valid_holding_register_addr(reg, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    handler.functions.write_holding_register(reg, value);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_write_holding_registers(uint8_t const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.write_holding_registers) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t first_reg = bytes_to_uint16_t(data);
    uint16_t n_registers = bytes_to_int16_t(data+2);
    uint16_t last_reg = first_reg + n_registers - 1;

    uint8_t n_values = ((uint8_t*)data)[4];

    if (n_values != (n_registers * 2)) { return EXCEPTION_ILLEGAL_DATA_ADDRESS; }

    if (!is_valid_holding_register_addr(first_reg, handler) || !is_valid_holding_register_addr(last_reg, handler))
    {
        return EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    copy_to_holding_registers(n_registers, data + 5, handler.data.write_holding_registers);

    handler.functions.write_holding_registers(first_reg, n_registers, handler.data.write_holding_registers);

    return EXCEPTION_NONE;
}

static MODBUS_EXCEPTION_CODES handle_read_write_registers(uint8_t const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.read_write_registers) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t read_start_reg = bytes_to_uint16_t(data);
    uint16_t n_read_count = bytes_to_uint16_t(data+2);
    uint16_t read_end_reg = read_start_reg + n_read_count - 1;
    
    uint16_t write_start_reg = bytes_to_uint16_t(data+4);
    uint16_t n_write_count = bytes_to_uint16_t(data+6);
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

    copy_to_holding_registers(n_write_count, data + 9, handler.data.write_holding_registers);

    handler.functions.read_write_registers(read_start_reg, n_read_count, write_start_reg, n_write_count, handler.data.write_holding_registers);

    return EXCEPTION_NONE;
}


static MODBUS_EXCEPTION_CODES handle_mask_write_register(uint8_t const * const data, const MODBUS_HANDLER& handler)
{
    if (!handler.functions.mask_write_register) { return EXCEPTION_ILLEGAL_FUNCTION_CODE; }

    uint16_t reg = bytes_to_uint16_t(data);
    uint16_t and_mask = bytes_to_uint16_t(data + 2);
    uint16_t or_mask = bytes_to_uint16_t(data + 4);

    if (!is_valid_holding_register_addr(reg, handler)) { return EXCEPTION_ILLEGAL_DATA_ADDRESS; }
    
    handler.functions.mask_write_register(reg, and_mask, or_mask);

    return EXCEPTION_NONE;
}

/*
 * Public Module Functions
 */

uint16_t modbus_get_crc16(uint8_t const * const buffer, uint8_t number_of_bytes)
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

bool modbus_validate_message_crc(const uint8_t * message, int message_length, bool reverse_order)
{
    bool valid_crc = true;

    if (application_check_crc(message, message_length, reverse_order) == CRC_PASSED) { return true; }

    uint8_t expected_hi;
    uint8_t expected_lo;
    
    uint16_t expected_crc = modbus_get_crc16(message, message_length - 2);
    
    expected_hi = reverse_order ? (uint8_t)(expected_crc & 0xFF) : (uint8_t)(expected_crc >> 8);
    expected_lo = reverse_order ? (uint8_t)(expected_crc >> 8) : (uint8_t)(expected_crc & 0xFF);
    
    valid_crc &= message[message_length-1] == expected_hi;
    valid_crc &= message[message_length-2] == expected_lo;
    
    return valid_crc;
}

void modbus_service_message(uint8_t const * const message, const MODBUS_HANDLER& handler, int message_length, bool check_crc)
{
    MODBUS_FUNCTION_CODE function_code;

    if (!message) { return; }

    uint8_t message_address = get_message_address(message);

    s_broadcast = (message_address == MODBUS_BROADCAST_ADDRESS);

    if (!s_broadcast && (message_address != handler.data.device_address)) { return; }
    if (!is_valid_function_code(message[1])) { return; }

    s_current_message = message;
    s_current_message_length = message_length;

    if (check_crc && !modbus_validate_message_crc(message, message_length))
    {
        if (handler.functions.exception_handler)
        {
            handler.functions.exception_handler(message[1]+128, EXCEPTION_INVALID_CRC);
        }
        return;
    }

    function_code = get_message_function_code(message);

    uint8_t const * const data_start = &message[2];

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
    default:
        break;
    }

    if ((exception != EXCEPTION_NONE) && (handler.functions.exception_handler))
    {
        handler.functions.exception_handler(function_code + 128, exception);    
    }

    s_current_message = NULL;
    s_current_message_length = 0;
}

uint8_t const * modbus_get_current_message()
{
    return s_current_message; 
}

uint8_t modbus_get_current_message_address()
{
    return get_message_address(s_current_message);
}

bool modbus_last_message_was_broadcast()
{
    return s_broadcast;
}

int modbus_get_current_message_length()
{
    return s_current_message_length;
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

int modbus_write_crc(uint8_t * const buffer, uint8_t bytes, bool reverse_order)
{
    uint16_t crc = modbus_get_crc16(buffer, bytes);
    uint8_t * crc_bytes = (uint8_t *)&crc;
    buffer[bytes] = crc_bytes[reverse_order ? 1 : 0];
    buffer[bytes+1] = crc_bytes[reverse_order ? 0 : 1];
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
    count += modbus_write(&buffer[count], (int8_t)(n_registers*2));
    
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

int modbus_write_read_holding_registers_response(uint8_t source_address, uint8_t * buffer, int16_t * holding_registers, uint8_t n_registers, bool add_crc)
{
    int count = 0;
    count += modbus_start_response(&buffer[count], READ_HOLDING_REGISTERS, source_address);
    count += modbus_write(&buffer[count], (int8_t)(n_registers*2));
    
    for (int i = 0; i < n_registers; i++)
    {
        count += modbus_write(&buffer[count], (int16_t)holding_registers[i]);
    }

    if (add_crc)
    {
        count += modbus_write_crc(buffer, count);
    }

    return count;
}

int modbus_get_write_single_coil_response(uint8_t source_address, uint8_t * buffer, uint16_t coil, bool on, bool add_crc)
{
    int count = 0;
    count += modbus_start_response(&buffer[count], WRITE_SINGLE_COIL, source_address);
    count += modbus_write(&buffer[count], (int16_t)coil);
    count += modbus_write(&buffer[count], on ? (int16_t)0xFF00 : (int16_t)0x0000);

    if (add_crc)
    {
        count += modbus_write_crc(buffer, count);
    }

    return count;
}

int modbus_get_write_holding_register_response(uint8_t source_address, uint8_t * buffer, uint16_t reg, int16_t value, bool add_crc)
{
    int count = 0;
    count += modbus_start_response(&buffer[count], WRITE_HOLDING_REGISTER, source_address);
    count += modbus_write(&buffer[count], (int16_t)reg);
    count += modbus_write(&buffer[count], (int16_t)value);

    if (add_crc)
    {
        count += modbus_write_crc(buffer, count);
    }

    return count;
}

int modbus_get_write_holding_registers_response(uint8_t source_address, uint8_t * buffer, uint16_t reg, uint16_t n_registers, bool add_crc)
{
    int count = 0;
    count += modbus_start_response(&buffer[count], WRITE_HOLDING_REGISTERS, source_address);
    count += modbus_write(&buffer[count], (int16_t)reg);
    count += modbus_write(&buffer[count], (int16_t)n_registers);

    if (add_crc)
    {
        count += modbus_write_crc(buffer, count);
    }

    return count;
}

int modbus_write_exception(uint8_t source_address, uint8_t * const buffer, MODBUS_EXCEPTION_CODES exception_code, uint8_t modified_function_code, bool add_crc)
{
    int count = 0;
    count += modbus_start_response(&buffer[count], (MODBUS_FUNCTION_CODE)modified_function_code, source_address);
    count += modbus_write(&buffer[count], (int8_t)exception_code);

    if (add_crc)
    {
        count += modbus_write_crc(buffer, count);
    }

    return count;
}
