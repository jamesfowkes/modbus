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

static void handle_read_coils(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	if (!handlers.read_coils) { return; }

	uint16_t first_coil = ((uint8_t)data[0] << 8) + (uint8_t)data[1];
	uint16_t n_coils = ((uint8_t)data[2] << 8) + (uint8_t)data[3];

	handlers.read_coils(first_coil, n_coils);
}

static void handle_read_discrete_inputs(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	(void)data;
	if (!handlers.read_discrete_inputs) { return; }
}

static void handle_write_single_coil(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	(void)data;
	if (!handlers.write_single_coil) { return; }
}

static void handle_write_multiple_coils(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	(void)data;
	if (!handlers.write_multiple_coils) { return; }
}

static void handle_read_input_registers(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	(void)data;
	if (!handlers.read_input_registers) { return; }
}

static void handle_read_holding_registers(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	(void)data;
	if (!handlers.read_holding_registers) { return; }
}

static void handle_write_holding_register(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	(void)data;
	if (!handlers.write_holding_register) { return; }
}

static void handle_write_holding_registers(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	(void)data;
	if (!handlers.write_holding_registers) { return; }
}

static void handle_read_write_registers(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	(void)data;
	if (!handlers.read_write_registers) { return; }
}


static void handle_mask_write_register(char const * const data, const MODBUS_HANDLER_FUNCTIONS& handlers)
{
	(void)data;
	if (!handlers.mask_write_register) { return; }
}
/*
 * Public Module Functions
 */

void modbus_init(uint8_t address)
{
	s_this_address = address;
}

void modbus_service_message(char const * const message, const MODBUS_HANDLER_FUNCTIONS& handlers)
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
		handle_read_coils(data_start, handlers);
		break;
	case READ_DISCRETE_INPUTS:
		handle_read_discrete_inputs(data_start, handlers);
		break;
	case WRITE_SINGLE_COIL:
		handle_write_single_coil(data_start, handlers);
		break;
	case WRITE_MULTIPLE_COILS:
		handle_write_multiple_coils(data_start, handlers);
		break;
	case READ_INPUT_REGISTERS:
		handle_read_input_registers(data_start, handlers);
		break;
	case READ_HOLDING_REGISTERS:
		handle_read_holding_registers(data_start, handlers);
		break;
	case WRITE_HOLDING_REGISTER:
		handle_write_holding_register(data_start, handlers);
		break;
	case WRITE_HOLDING_REGISTERS:
		handle_write_holding_registers(data_start, handlers);
		break;
	case READ_WRITE_REGISTERS:
		handle_read_write_registers(data_start, handlers);
		break;
	case MASK_WRITE_REGISTER:
		handle_mask_write_register(data_start, handlers);
		break;
	}
}

