#include <stdint.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "modbus.h"

static const int NUMBER_OF_COILS = 6;
static const int NUMBER_OF_INPUTS = 9;
static const int NUMBER_OF_INPUT_REGISTERS = 4;
static const int NUMBER_OF_HOLDING_REGISTERS = 6;

static uint8_t s_last_exception_function;
static MODBUS_EXCEPTION_CODES s_last_exception_code;
static uint8_t s_test_data_buffer[256];

static MODBUS_HANDLER s_modbus_handler;

static bool test_buffer_is_empty()
{
	int i;
	bool empty = true;
	for (i = 0; i < 256; i++)
	{
		empty &= (s_test_data_buffer[i] == UINT8_MAX);
	}
	return empty;
}

static void read_coils_function(uint16_t, uint16_t) {}
static void read_discrete_inputs_function(uint16_t, uint16_t) {}
static void write_single_coil_function(uint16_t, bool) {}
static void write_multiple_coils_function(uint16_t, uint16_t, bool*) {}
static void read_input_registers_function(uint16_t, uint16_t) {}
static void read_holding_registers_function(uint16_t, uint16_t) {}
static void write_holding_register_function(uint16_t, int16_t) {}
static void write_holding_registers_function(uint16_t, uint16_t, int16_t*) {}
static void read_write_registers_function(uint16_t, uint16_t, uint16_t, uint16_t, int16_t*) {}
static void mask_write_register_function(uint16_t, uint16_t, uint16_t) {}

static void exception_handler(uint8_t function_code, MODBUS_EXCEPTION_CODES exception_code)
{
	s_last_exception_function = function_code;
	s_last_exception_code = exception_code;
}

class ModbusExceptionTest : public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(ModbusExceptionTest);

	CPPUNIT_TEST(test_service_calls_exception_callback_with_read_coils_illegal_function_exception);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_read_discrete_inputs_illegal_function_exception);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_write_single_coil_illegal_function_exception);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_write_multiple_coils_illegal_function_exception);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_read_input_registers_illegal_function_exception);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_read_holding_registers_illegal_function_exception);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_write_holding_register_illegal_function_exception);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_write_holding_registers_illegal_function_exception);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_read_write_registers_illegal_function_exception);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_mask_write_register_illegal_function_exception);
	
	CPPUNIT_TEST(test_service_calls_exception_handler_with_illegal_read_coils_data_length);
	CPPUNIT_TEST(test_service_calls_exception_handler_with_illegal_read_coils_data_address_too_high);
	CPPUNIT_TEST(test_service_calls_exception_handler_with_illegal_read_coils_data_address_zero);

	CPPUNIT_TEST(test_service_calls_exception_handler_with_illegal_read_discrete_inputs_data_length);
	CPPUNIT_TEST(test_service_calls_exception_handler_with_illegal_read_discrete_inputs_data_address_too_high);
	CPPUNIT_TEST(test_service_calls_exception_handler_with_illegal_read_discrete_inputs_data_address_zero);

	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_single_coil_data_address_zero);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_single_coil_data_address_too_high);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_single_coil_data_value);

	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_multiple_coils_data_length);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_multiple_coils_data_address_too_high);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_multiple_coils_data_address_zero);

	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_input_registers_data_length);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_input_registers_data_address_too_high);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_input_registers_data_address_zero);

	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_holding_registers_data_length);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_holding_registers_data_address_too_high);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_holding_registers_data_address_zero);

	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_holding_register_data_address_too_high);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_holding_register_data_address_zero);

	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_holding_registers_data_length);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_holding_registers_data_address_too_high);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_holding_registers_data_address_zero);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_write_holding_registers_data_counts);

	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_read_length);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_read_address);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_write_length);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_write_address);
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_write_counts);
	
	CPPUNIT_TEST(test_service_calls_exception_callback_with_illegal_mask_write_register_data_address_too_high);

	CPPUNIT_TEST_SUITE_END();

	void test_service_calls_exception_callback_with_read_coils_illegal_function_exception()
	{
		char message[] = {(char)0xAA, READ_COILS};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_COILS), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_callback_with_read_discrete_inputs_illegal_function_exception()
	{
		char message[] = {(char)0xAA, READ_DISCRETE_INPUTS};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_DISCRETE_INPUTS), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_callback_with_write_single_coil_illegal_function_exception()
	{
		char message[] = {(char)0xAA, WRITE_SINGLE_COIL};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_SINGLE_COIL), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_callback_with_write_multiple_coils_illegal_function_exception()
	{
		char message[] = {(char)0xAA, WRITE_MULTIPLE_COILS};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_MULTIPLE_COILS), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_callback_with_read_input_registers_illegal_function_exception()
	{
		char message[] = {(char)0xAA, READ_INPUT_REGISTERS};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_INPUT_REGISTERS), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_callback_with_read_holding_registers_illegal_function_exception()
	{

		char message[] = {(char)0xAA, READ_HOLDING_REGISTERS};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_HOLDING_REGISTERS), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_callback_with_write_holding_register_illegal_function_exception()
	{

		char message[] = {(char)0xAA, WRITE_HOLDING_REGISTER};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_HOLDING_REGISTER), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_callback_with_write_holding_registers_illegal_function_exception()
	{
		char message[] = {(char)0xAA, WRITE_HOLDING_REGISTERS};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_HOLDING_REGISTERS), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_callback_with_read_write_registers_illegal_function_exception()
	{

		char message[] = {(char)0xAA, READ_WRITE_REGISTERS};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_WRITE_REGISTERS), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_callback_with_mask_write_register_illegal_function_exception()
	{

		char message[] = {(char)0xAA, MASK_WRITE_REGISTER};
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + MASK_WRITE_REGISTER), s_last_exception_function);	
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_FUNCTION_CODE, s_last_exception_code);
	}

	void test_service_calls_exception_handler_with_illegal_read_coils_data_length()
	{
		char message[] = {(char)0xAA, (char)READ_COILS, (char)0x00, (char)0x00, (char)0x00, (char)NUMBER_OF_COILS+1};
		s_modbus_handler.functions.read_coils = read_coils_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_COILS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_handler_with_illegal_read_coils_data_address_zero()
	{
		char message[] = {(char)0xAA, (char)READ_COILS, (char)0x00, (char)0x00, (char)0x00, (char)0x01};
		s_modbus_handler.functions.read_coils = read_coils_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_COILS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_handler_with_illegal_read_coils_data_address_too_high()
	{
		char message[] = {(char)0xAA, (char)READ_COILS, (char)0x00, (char)NUMBER_OF_COILS+1, (char)0x00, (char)0x01};
		s_modbus_handler.functions.read_coils = read_coils_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_COILS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_handler_with_illegal_read_discrete_inputs_data_length()
	{
		char message[] = {(char)0xAA, (char)READ_DISCRETE_INPUTS, (char)0x00, (char)0x00, (char)0x00, (char)NUMBER_OF_INPUTS+1};
		s_modbus_handler.functions.read_discrete_inputs = read_discrete_inputs_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_DISCRETE_INPUTS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_handler_with_illegal_read_discrete_inputs_data_address_too_high()
	{
		char message[] = {(char)0xAA, (char)READ_DISCRETE_INPUTS, (char)0x00, (char)NUMBER_OF_INPUTS+1, (char)0x00, (char)0x01};
		s_modbus_handler.functions.read_discrete_inputs = read_discrete_inputs_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_DISCRETE_INPUTS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_handler_with_illegal_read_discrete_inputs_data_address_zero()
	{
		char message[] = {(char)0xAA, (char)READ_DISCRETE_INPUTS, (char)0x00, (char)0x00, (char)0x00, (char)0x01};
		s_modbus_handler.functions.read_discrete_inputs = read_discrete_inputs_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_DISCRETE_INPUTS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_single_coil_data_address_zero()
	{
		char message[] = {(char)0xAA, WRITE_SINGLE_COIL, (char)0x00, (char)0x00, (char)0x00, (char)0x00};
		s_modbus_handler.functions.write_single_coil = write_single_coil_function;
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_SINGLE_COIL), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_single_coil_data_address_too_high()
	{
		char message[] = {(char)0xAA, WRITE_SINGLE_COIL, (char)0x00, (char)NUMBER_OF_COILS+1, (char)0x00, (char)0x00};
		s_modbus_handler.functions.write_single_coil = write_single_coil_function;
		modbus_service_message(message, s_modbus_handler, 0, false);			
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_SINGLE_COIL), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_single_coil_data_value()
	{
		char message[] = {(char)0xAA, WRITE_SINGLE_COIL, (char)0x00, (char)NUMBER_OF_COILS-1, (char)0x00, (char)0x01};
		s_modbus_handler.functions.write_single_coil = write_single_coil_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_SINGLE_COIL), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_VALUE, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_multiple_coils_data_length()
	{
		char message[] = {(char)0xAA, WRITE_MULTIPLE_COILS, (char)0x00, (char)0x00, (char)0x00, (char)NUMBER_OF_COILS+1};
		s_modbus_handler.functions.write_multiple_coils = write_multiple_coils_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_MULTIPLE_COILS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_multiple_coils_data_address_too_high()
	{
		char message[] = {(char)0xAA, WRITE_MULTIPLE_COILS, (char)0x00, (char)NUMBER_OF_COILS+1, (char)0x00, (char)0x01};
		s_modbus_handler.functions.write_multiple_coils = write_multiple_coils_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_MULTIPLE_COILS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_multiple_coils_data_address_zero()
	{
		char message[] = {(char)0xAA, WRITE_MULTIPLE_COILS, (char)0x00, (char)0x00, (char)0x00, (char)0x01};
		s_modbus_handler.functions.write_multiple_coils = write_multiple_coils_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_MULTIPLE_COILS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_read_input_registers_data_length()
	{
		char message[] = {(char)0xAA, (char)READ_INPUT_REGISTERS, (char)0x00, (char)0x00, (char)0x00, (char)NUMBER_OF_INPUT_REGISTERS+1};
		s_modbus_handler.functions.read_input_registers = read_input_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_INPUT_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_read_input_registers_data_address_too_high()
	{
		char message[] = {(char)0xAA, (char)READ_INPUT_REGISTERS, (char)0x00, (char)NUMBER_OF_INPUT_REGISTERS+1, (char)0x00, (char)0x01};
		s_modbus_handler.functions.read_input_registers = read_input_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_INPUT_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_read_input_registers_data_address_zero()
	{
		char message[] = {(char)0xAA, (char)READ_INPUT_REGISTERS, (char)0x00, (char)0x00, (char)0x00, (char)0x01};
		s_modbus_handler.functions.read_input_registers = read_input_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_INPUT_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_read_holding_registers_data_length()
	{
		char message[] = {(char)0xAA, (char)READ_HOLDING_REGISTERS, (char)0x00, (char)0x00, (char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS+1};
		s_modbus_handler.functions.read_holding_registers = read_holding_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_HOLDING_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_read_holding_registers_data_address_too_high()
	{
		char message[] = {(char)0xAA, (char)READ_HOLDING_REGISTERS, (char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS+1, (char)0x00, (char)0x01};
		s_modbus_handler.functions.read_holding_registers = read_holding_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_HOLDING_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_read_holding_registers_data_address_zero()
	{
		char message[] = {(char)0xAA, (char)READ_HOLDING_REGISTERS, (char)0x00, (char)0x00, (char)0x00, (char)0x01};
		s_modbus_handler.functions.read_holding_registers = read_holding_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_HOLDING_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_holding_register_data_address_too_high()
	{
		char message[] = {(char)0xAA, (char)WRITE_HOLDING_REGISTER, (char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS+1, (char)0x00, (char)0x00};
		s_modbus_handler.functions.write_holding_register = write_holding_register_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_HOLDING_REGISTER), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());	
	}

	void test_service_calls_exception_callback_with_illegal_write_holding_register_data_address_zero()
	{
		char message[] = {(char)0xAA, (char)WRITE_HOLDING_REGISTER, (char)0x00, (char)0x00, (char)0x00, (char)0x00};
		s_modbus_handler.functions.write_holding_register = write_holding_register_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_HOLDING_REGISTER), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());	
	}

	void test_service_calls_exception_callback_with_illegal_write_holding_registers_data_length()
	{
		char message[] = {(char)0xAA, (char)WRITE_HOLDING_REGISTERS, 
			(char)0x00, (char)0x00,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS+1,
			(char)((NUMBER_OF_HOLDING_REGISTERS+1)*2)
		};

		s_modbus_handler.functions.write_holding_registers = write_holding_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_HOLDING_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_holding_registers_data_address_too_high()
	{
		char message[] = {(char)0xAA, (char)WRITE_HOLDING_REGISTERS, 
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS+1,
			(char)0x00, (char)0x01,
			(char)0x02
		};
		s_modbus_handler.functions.write_holding_registers = write_holding_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_HOLDING_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_holding_registers_data_address_zero()
	{
		char message[] = {(char)0xAA, (char)WRITE_HOLDING_REGISTERS, 
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS+1,
			(char)0x00, (char)0x01,
			(char)0x02
		};
		s_modbus_handler.functions.write_holding_registers = write_holding_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_HOLDING_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_write_holding_registers_data_counts()
	{
		char message[] = {(char)0xAA, (char)WRITE_HOLDING_REGISTERS, 
			(char)0x00, (char)0x00,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS,
			(char)0x00,
		};

		s_modbus_handler.functions.write_holding_registers = write_holding_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + WRITE_HOLDING_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());
	}

	void test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_read_length()
	{
		char message[] = {
			(char)0xAA, (char)READ_WRITE_REGISTERS, 
			(char)0x00, (char)0x00,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS+1,
			(char)0x00, (char)0x00,
			(char)0x00, (char)0x01,
			(char)0x02,
			(char)0x00, (char)0x00,	
		};

		s_modbus_handler.functions.read_write_registers = read_write_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_WRITE_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());	
	}

	void test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_read_address()
	{
		char message[] = {
			(char)0xAA, (char)READ_WRITE_REGISTERS, 
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS,
			(char)0x00, (char)0x01,
			(char)0x00, (char)0x00,
			(char)0x00, (char)0x01,
			(char)0x02,
			(char)0x00, (char)0x00,	
		};

		s_modbus_handler.functions.read_write_registers = read_write_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_WRITE_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());	
	}

	void test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_write_length()
	{
		char message[] = {
			(char)0xAA, (char)READ_WRITE_REGISTERS, 
			(char)0x00, (char)0x00,
			(char)0x00, (char)0x01,
			(char)0x00, (char)0x00,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS+1,
			(char)0x02,
			(char)0x00, (char)0x00,	
		};

		s_modbus_handler.functions.read_write_registers = read_write_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_WRITE_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());	
	}

	void test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_write_address()
	{
		char message[] = {
			(char)0xAA, (char)READ_WRITE_REGISTERS, 
			(char)0x00, (char)0x00,
			(char)0x00, (char)0x01,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS,
			(char)0x00, (char)0x01,
			(char)0x02,
			(char)0x00, (char)0x00,	
		};

		s_modbus_handler.functions.read_write_registers = read_write_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_WRITE_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());	
	}

	void test_service_calls_exception_callback_with_illegal_read_write_input_registers_bad_write_counts()
	{
		char message[] = {
			(char)0xAA, (char)READ_WRITE_REGISTERS, 
			(char)0x00, (char)0x00,
			(char)0x00, (char)0x01,
			(char)0x00, (char)0x00,
			(char)0x00, (char)0x01,
			(char)0x01,
			(char)0x00, (char)0x00,	
		};

		s_modbus_handler.functions.read_write_registers = read_write_registers_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + READ_WRITE_REGISTERS), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());	
	}	

	void test_service_calls_exception_callback_with_illegal_mask_write_register_data_address_too_high()
	{
		char message[] = {
			(char)0xAA, (char)MASK_WRITE_REGISTER,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS+1,
			(char)0x00, (char)0x00,
			(char)0x00, (char)0x00,
			(char)0x00, (char)0x00
		};

		s_modbus_handler.functions.mask_write_register = mask_write_register_function;
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((uint8_t)(128 + MASK_WRITE_REGISTER), s_last_exception_function);
		CPPUNIT_ASSERT_EQUAL(EXCEPTION_ILLEGAL_DATA_ADDRESS, s_last_exception_code);
		CPPUNIT_ASSERT(test_buffer_is_empty());	
	}

public:
	void setUp()
	{
		s_modbus_handler.functions.read_coils = NULL;
		s_modbus_handler.functions.read_discrete_inputs = NULL;
		s_modbus_handler.functions.write_single_coil = NULL;
		s_modbus_handler.functions.write_multiple_coils = NULL;
		s_modbus_handler.functions.read_input_registers = NULL;
		s_modbus_handler.functions.read_holding_registers = NULL;
		s_modbus_handler.functions.write_holding_register = NULL;
		s_modbus_handler.functions.write_holding_registers = NULL;
		s_modbus_handler.functions.read_write_registers = NULL;
		s_modbus_handler.functions.mask_write_register = NULL;
		s_modbus_handler.functions.exception_handler = exception_handler;
		s_modbus_handler.data.device_address = 0xAA;
		s_modbus_handler.data.write_multiple_coils = (bool*)s_test_data_buffer; 
		s_modbus_handler.data.write_holding_registers = (int16_t*)s_test_data_buffer;
		s_modbus_handler.data.num_coils = NUMBER_OF_COILS;
		s_modbus_handler.data.num_inputs =  NUMBER_OF_INPUTS;
		s_modbus_handler.data.num_input_registers = NUMBER_OF_INPUT_REGISTERS;
		s_modbus_handler.data.num_holding_registers = NUMBER_OF_HOLDING_REGISTERS;

		s_last_exception_function = 0;
		s_last_exception_code = (MODBUS_EXCEPTION_CODES)0xFF;

		int i;
		for (i = 0; i < 256; i++)
		{
			s_test_data_buffer[i] = UINT8_MAX;
		}

	}
};


int main()
{
   CppUnit::TextUi::TestRunner runner;
   
   CPPUNIT_TEST_SUITE_REGISTRATION( ModbusExceptionTest );

   CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();

   runner.addTest( registry.makeTest() );
   runner.run();

   return 0;
}
