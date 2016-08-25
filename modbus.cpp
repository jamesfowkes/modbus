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
 * Private Module Variables
 */

static uint8_t s_this_address = 0x00;

/*
 * Private Module Functions
 */

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

static void copy_to_coils(uint16_t n_coils, uint8_t *data, uint8_t * coils)
{
	uint16_t i;
	for (i = 0; i < n_coils; i++)
	{
		coils[i] = data[i];
	}	
}

static bool bytes_to_on_off_data(uint8_t * bytes)
{
	return bytes[0] == 0xFF;
}

static uint8_t get_number_of_required_bytes_for_coils(uint16_t n_coils)
{
	return (n_coils & 7) ? (n_coils / 8) + 1 : n_coils / 8;
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

static void handle_read_coils(void const * const data, const MODBUS_HANDLER& handler)
{
	if (!handler.functions.read_coils) { return; }

	uint16_t first_coil = bytes_to_uint16_t((uint8_t*)data);
	uint16_t n_coils = bytes_to_uint16_t((uint8_t*)data+2);

	handler.functions.read_coils(first_coil, n_coils);
}

static void handle_read_discrete_inputs(void const * const data, const MODBUS_HANDLER& handler)
{
	if (!handler.functions.read_discrete_inputs) { return; }

	uint16_t first_input = bytes_to_uint16_t((uint8_t*)data);
	uint16_t n_inputs = bytes_to_uint16_t((uint8_t*)data+2);

	handler.functions.read_discrete_inputs(first_input, n_inputs);
}

static bool on_off_data_is_valid(uint8_t * data)
{
	bool valid_on_off_data = false;
	valid_on_off_data |= (data[0] == 0xFF) && (data[1] == 0x00);
	valid_on_off_data |= (data[0] == 0x00) && (data[1] == 0x00);
	return valid_on_off_data;
}

static void handle_write_single_coil(void const * const data, const MODBUS_HANDLER& handler)
{
	if (!handler.functions.write_single_coil) { return; }

	if (!on_off_data_is_valid((uint8_t*)data + 2)) { return; }

	uint16_t coil = bytes_to_uint16_t((uint8_t*)data);
	bool on = bytes_to_on_off_data((uint8_t*)data + 2);

	handler.functions.write_single_coil(coil, on);
}

static void handle_write_multiple_coils(char const * const data, const MODBUS_HANDLER& handler)
{
	if (!handler.functions.write_multiple_coils) { return; }

	uint16_t first_coil = bytes_to_uint16_t((uint8_t*)data);
	uint16_t n_coils = bytes_to_uint16_t((uint8_t*)data + 2);
	uint8_t n_values = data[4];
	
	if (n_coils > handler.data.max_coils)
	{
		n_coils = handler.data.max_coils;
		n_values = get_number_of_required_bytes_for_coils(n_coils);
	}

	copy_to_coils(n_coils, (uint8_t*)data + 5, handler.data.write_multiple_coils);
	handler.functions.write_multiple_coils(first_coil, n_coils, n_values, handler.data.write_multiple_coils);
}

static void handle_read_input_registers(char const * const data, const MODBUS_HANDLER& handler)
{
	if (!handler.functions.read_input_registers) { return; }

	uint16_t reg = bytes_to_uint16_t((uint8_t*)data);
	uint16_t n_registers = bytes_to_uint16_t((uint8_t*)data+2);

	handler.functions.read_input_registers(reg, n_registers);
}

static void handle_read_holding_registers(char const * const data, const MODBUS_HANDLER& handler)
{
	if (!handler.functions.read_holding_registers) { return; }

	uint16_t reg = bytes_to_uint16_t((uint8_t*)data);
	uint16_t n_registers = bytes_to_uint16_t((uint8_t*)data+2);

	handler.functions.read_holding_registers(reg, n_registers);
}

static void handle_write_holding_register(char const * const data, const MODBUS_HANDLER& handler)
{
	if (!handler.functions.write_holding_register) { return; }

	uint16_t reg = bytes_to_uint16_t((uint8_t*)data);
	int16_t value = bytes_to_int16_t((uint8_t*)data+2);

	handler.functions.write_holding_register(reg, value);
}

static void handle_write_holding_registers(char const * const data, const MODBUS_HANDLER& handler)
{
	if (!handler.functions.write_holding_registers) { return; }

	uint16_t first_reg = bytes_to_uint16_t((uint8_t*)data);
	uint16_t n_registers = bytes_to_int16_t((uint8_t*)data+2);
	uint8_t n_values = ((uint8_t*)data)[4];
	
	if (n_registers > handler.data.max_holding_registers)
	{
		n_registers = handler.data.max_holding_registers;
		n_values = n_registers * 2;
	}

	copy_to_holding_registers(n_registers, (uint8_t*)data + 5, handler.data.write_holding_registers);

	handler.functions.write_holding_registers(first_reg, n_registers, n_values, handler.data.write_holding_registers);
}

static void handle_read_write_registers(char const * const data, const MODBUS_HANDLER& handler)
{
	if (!handler.functions.read_write_registers) { return; }

	uint16_t read_start_reg = bytes_to_uint16_t((uint8_t*)data);
	uint16_t n_read_count = bytes_to_uint16_t((uint8_t*)data+2);
	uint16_t write_start_reg = bytes_to_uint16_t((uint8_t*)data+4);
	uint16_t n_write_count = bytes_to_uint16_t((uint8_t*)data+6);

	if (n_write_count > handler.data.max_holding_registers)
	{
		n_write_count = handler.data.max_holding_registers;
	}

	copy_to_holding_registers(n_write_count, (uint8_t*)data + 8, handler.data.write_holding_registers);

	handler.functions.read_write_registers(read_start_reg, n_read_count, write_start_reg, n_write_count, handler.data.write_holding_registers);
}


static void handle_mask_write_register(char const * const data, const MODBUS_HANDLER& handler)
{
	(void)data;
	if (!handler.functions.mask_write_register) { return; }

	uint16_t reg = bytes_to_uint16_t((uint8_t*)data);
	uint16_t and_mask = bytes_to_uint16_t((uint8_t*)data + 2);
	uint16_t or_mask = bytes_to_uint16_t((uint8_t*)data + 4);

	handler.functions.mask_write_register(reg, and_mask, or_mask);
}
/*
 * Public Module Functions
 */

void modbus_init(uint8_t address)
{
	s_this_address = address;
}

void modbus_service_message(char const * const message, const MODBUS_HANDLER& handler)
{
	
	MODBUS_FUNCTION_CODE function_code;

	if (!message) { return; }

	if (get_message_address(message) != s_this_address) { return; }

	if (!is_valid_function_code(message[1])) { return; }

	function_code = get_message_function_code(message);

	char const * const data_start = &message[2];

	switch(function_code)
	{
	case READ_COILS:
		handle_read_coils(data_start, handler);
		break;
	case READ_DISCRETE_INPUTS:
		handle_read_discrete_inputs(data_start, handler);
		break;
	case WRITE_SINGLE_COIL:
		handle_write_single_coil(data_start, handler);
		break;
	case WRITE_MULTIPLE_COILS:
		handle_write_multiple_coils(data_start, handler);
		break;
	case READ_INPUT_REGISTERS:
		handle_read_input_registers(data_start, handler);
		break;
	case READ_HOLDING_REGISTERS:
		handle_read_holding_registers(data_start, handler);
		break;
	case WRITE_HOLDING_REGISTER:
		handle_write_holding_register(data_start, handler);
		break;
	case WRITE_HOLDING_REGISTERS:
		handle_write_holding_registers(data_start, handler);
		break;
	case READ_WRITE_REGISTERS:
		handle_read_write_registers(data_start, handler);
		break;
	case MASK_WRITE_REGISTER:
		handle_mask_write_register(data_start, handler);
		break;
	}
}

