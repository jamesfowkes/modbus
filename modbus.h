#ifndef _MODBUS_H_
#define _MODBUS_H_

struct modbus_handler_functions
{
	void (*read_coils)(uint16_t first_coil, uint16_t n_coils);
	void (*read_discrete_inputs)(uint16_t first_input, uint16_t n_inputs);
	void (*write_single_coil)(uint16_t coil, bool on);
	void (*write_multiple_coils)(uint16_t first_coil, uint16_t n_coils, uint8_t n_values, uint8_t * values);
	void (*read_input_registers)(uint16_t reg, uint16_t n_registers);
	void (*read_holding_registers)(uint16_t reg, uint16_t n_registers);
	void (*write_holding_register)(uint16_t reg, int16_t value);
	void (*write_holding_registers)(uint16_t first_reg, uint16_t n_registers, uint8_t n_values, int16_t * values);
	void (*read_write_registers)(uint16_t read_start_reg, uint16_t n_registers, uint16_t write_start_reg, uint16_t n_values, int16_t * values);
	void (*mask_write_register)(uint16_t reg, uint16_t and_mask, uint16_t or_mask);
};

struct modbus_handler_data
{
	uint8_t * write_multiple_coils;
	uint16_t max_coils;

	int16_t * write_holding_registers;
	uint16_t max_holding_registers;
};

struct modbus_handler
{
	struct modbus_handler_functions functions;
	struct modbus_handler_data data;
};
typedef struct modbus_handler MODBUS_HANDLER;

enum modbus_function_code
{
	READ_COILS = 1,
	READ_DISCRETE_INPUTS = 2,
	WRITE_SINGLE_COIL = 5,
	WRITE_MULTIPLE_COILS = 15,
	READ_INPUT_REGISTERS = 4,
	READ_HOLDING_REGISTERS = 3,
	WRITE_HOLDING_REGISTER = 6,
	WRITE_HOLDING_REGISTERS = 16,
	READ_WRITE_REGISTERS = 23,
	MASK_WRITE_REGISTER = 22
};
typedef enum modbus_function_code MODBUS_FUNCTION_CODE;

void modbus_init(uint8_t address);

void modbus_service_message(char const * const message, const MODBUS_HANDLER& handler);

#endif