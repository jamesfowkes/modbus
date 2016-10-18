#ifndef _MODBUS_H_
#define _MODBUS_H_

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

enum modbus_exception_codes
{
	EXCEPTION_NONE,
	EXCEPTION_ILLEGAL_FUNCTION_CODE = 1,
	EXCEPTION_ILLEGAL_DATA_ADDRESS = 2,
	EXCEPTION_ILLEGAL_DATA_VALUE = 3,
	EXCEPTION_SLAVE_DEVICE_FAILURE =4 ,
	EXCEPTION_ACKNOWLEDGE = 5,
	EXCEPTION_SLAVE_DEVICE_BUSY = 6,
	EXCEPTION_NEGATIVE_ACKNOWLEDGE = 7,
	EXCEPTION_MEMORY_PARITY_ERR = 8,
	EXCEPTION_GATEWAY_PATH_UNAVAILABLE = 10,
	EXCEPTION_GATEWAY_TGT_DEVICE_NO_RSP = 11,
	EXCEPTION_INVALID_CRC = 127,
};
typedef enum modbus_exception_codes MODBUS_EXCEPTION_CODES;

struct modbus_handler_functions
{
	void (*read_coils)(uint16_t first_coil, uint16_t n_coils);
	void (*read_discrete_inputs)(uint16_t first_input, uint16_t n_inputs);
	void (*write_single_coil)(uint16_t coil, bool on);
	void (*write_multiple_coils)(uint16_t first_coil, uint16_t n_coils, bool * values);
	void (*read_input_registers)(uint16_t reg, uint16_t n_registers);
	void (*read_holding_registers)(uint16_t reg, uint16_t n_registers);
	void (*write_holding_register)(uint16_t reg, int16_t value);
	void (*write_holding_registers)(uint16_t first_reg, uint16_t n_registers, int16_t * values);
	void (*read_write_registers)(uint16_t read_start_reg, uint16_t n_registers, uint16_t write_start_reg, uint16_t n_values, int16_t * values);
	void (*mask_write_register)(uint16_t reg, uint16_t and_mask, uint16_t or_mask);

	void (*exception_handler)(uint8_t function_code, MODBUS_EXCEPTION_CODES exception_code);
};

struct modbus_handler_data
{
	uint8_t device_address;
	
	uint16_t num_coils;
	uint16_t num_inputs;
	uint16_t num_input_registers;
	uint16_t num_holding_registers;

	bool * write_multiple_coils;	
	int16_t * write_holding_registers;
};

struct modbus_handler
{
	struct modbus_handler_functions functions;
	struct modbus_handler_data data;
	bool add_response_crc;
};
typedef struct modbus_handler MODBUS_HANDLER;

void modbus_service_message(char const * const message, const MODBUS_HANDLER& handler, int message_length, bool check_crc);

int modbus_start_response(uint8_t * const buffer, MODBUS_FUNCTION_CODE function_code, uint8_t device_address);

int modbus_write(uint8_t * const buffer, int8_t value);
int modbus_write(uint8_t * const buffer, int16_t value);
int modbus_write_crc(uint8_t * const buffer, uint8_t bytes);
int modbus_write_read_discrete_inputs_response(uint8_t source_address, uint8_t * buffer, bool * discrete_inputs, uint8_t n_inputs, bool add_crc=true);
int modbus_write_read_input_registers_response(uint8_t source_address, uint8_t * buffer, int16_t * input_registers, uint8_t n_registers, bool add_crc=true);
int modbus_write_read_holding_registers_response(uint8_t source_address, uint8_t * buffer, int16_t * holding_registers, uint8_t n_registers, bool add_crc=true);
int modbus_get_write_single_coil_response(uint8_t source_address, uint8_t * buffer, uint16_t coil, bool on, bool add_crc=true);
int modbus_get_write_holding_register_response(uint8_t source_address, uint8_t * buffer, uint16_t reg, int16_t value, bool add_crc=true);
int modbus_get_write_holding_registers_response(uint8_t source_address, uint8_t * buffer, uint16_t reg, uint16_t n_registers, bool add_crc=true);

#endif