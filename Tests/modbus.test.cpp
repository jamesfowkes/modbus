#include <stdint.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "modbus.h"

static int s_last_function_code;

static struct _read_coils_data {uint16_t first_coil; uint16_t n_coils;} s_read_coils_data;
static void read_coils(uint16_t first_coil, uint16_t n_coils)
{
	s_read_coils_data.first_coil = first_coil;
	s_read_coils_data.n_coils = n_coils;
	s_last_function_code = READ_COILS;
}

static struct _read_discrete_inputs_data {uint16_t first_input; uint16_t n_inputs;} s_read_discrete_inputs_data;
static void read_discrete_inputs(uint16_t first_input, uint16_t n_inputs)
{
	s_read_discrete_inputs_data.first_input = first_input;
	s_read_discrete_inputs_data.n_inputs = n_inputs;
	s_last_function_code = READ_DISCRETE_INPUTS;
}

static struct _write_single_coil_data {uint16_t coil; bool on;} s_write_single_coil_data;
static void write_single_coil(uint16_t coil, bool on)
{
	s_write_single_coil_data.coil = coil;
	s_write_single_coil_data.on = on;
	s_last_function_code = WRITE_SINGLE_COIL;
}

//static void write_multiple_coils(uint16_t first_coil, uint16_t n_coils, uint8_t n_values, uint8_t * values)
//{
//	s_last_function_code = WRITE_MULTIPLE_COILS;
//}
//
//static void read_input_registers(uint16_t reg, uint16_t n_registers)
//{
//	s_last_function_code = READ_INPUT_REGISTERS;
//}
//
//static void read_holding_registers(uint16_t reg, uint16_t n_registers)
//{
//	s_last_function_code = READ_HOLDING_REGISTERS;
//}
//
//static void write_holding_register(uint16_t reg, int16_t value)
//{
//	s_last_function_code = WRITE_HOLDING_REGISTER;
//}
//
//static void write_holding_registers(uint16_t first_reg, uint16_t n_registers, uint8_t n_values, int16_t * values)
//{
//	s_last_function_code = WRITE_HOLDING_REGISTERS;
//}
//
//static void read_write_registers(uint16_t read_start_reg, uint16_t n_registers, uint16_t write_start_reg, uint16_t n_values, uint16_t * int16_t)
//{
//	s_last_function_code = READ_WRITE_REGISTERS;
//}
//
//static void mask_write_register(uint16_t reg, uint16_t and_mask, uint16_t or_mask)
//{
//	s_last_function_code = MASK_WRITE_REGISTER;
//}

static const MODBUS_HANDLER_FUNCTIONS s_handler_functions = {
	read_coils,
	read_discrete_inputs,
	write_single_coil,
	NULL,//write_multiple_coils,
	NULL,//read_input_registers,
	NULL,//read_holding_registers,
	NULL,//write_holding_register,
	NULL,//write_holding_registers,
	NULL,//read_write_registers,
	NULL,//mask_write_register
};

class ModbusTest : public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(ModbusTest);

	CPPUNIT_TEST(test_service_with_no_message_does_not_call_any_functions);
	CPPUNIT_TEST(test_service_with_wrong_address_does_not_call_any_functions);
	CPPUNIT_TEST(test_service_with_read_coils_message);
	CPPUNIT_TEST(test_service_with_read_discrete_inputs_message);
	CPPUNIT_TEST(test_service_with_invalid_write_single_coil_message);
	CPPUNIT_TEST(test_service_with_valid_write_single_coil_message);

	CPPUNIT_TEST_SUITE_END();

	void test_service_with_no_message_does_not_call_any_functions()
	{
		char message[] = {(char)0xAB};
		modbus_service_message(message, s_handler_functions);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_wrong_address_does_not_call_any_functions()
	{
		modbus_service_message(NULL, s_handler_functions);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_read_coils_message()
	{
		char message[] = {(char)0xAA, (char)READ_COILS, (char)0x80, (char)0xFF, (char)0x80, (char)0xF1};

		modbus_service_message(message, s_handler_functions);
		CPPUNIT_ASSERT_EQUAL((int)READ_COILS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80FF, s_read_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80F1, s_read_coils_data.n_coils);
	}

	void test_service_with_read_discrete_inputs_message()
	{
		char message[] = {(char)0xAA, (char)READ_DISCRETE_INPUTS, (char)0x80, (char)0xFF, (char)0x80, (char)0xF1};

		modbus_service_message(message, s_handler_functions);
		CPPUNIT_ASSERT_EQUAL((int)READ_DISCRETE_INPUTS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80FF, s_read_discrete_inputs_data.first_input);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80F1, s_read_discrete_inputs_data.n_inputs);
	}

	void test_service_with_invalid_write_single_coil_message()
	{
		char message[] = {(char)0xAA, (char)WRITE_SINGLE_COIL, (char)0x80, (char)0xFF, (char)0x00, (char)0x01};

		modbus_service_message(message, s_handler_functions);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_valid_write_single_coil_message()
	{
		char message[] = {(char)0xAA, (char)WRITE_SINGLE_COIL, (char)0x80, (char)0xFF, (char)0xFF, (char)0x00};

		modbus_service_message(message, s_handler_functions);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_SINGLE_COIL, s_last_function_code);

		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80FF, s_write_single_coil_data.coil);
		CPPUNIT_ASSERT(s_write_single_coil_data.on);
	}

public:
	void setUp()
	{
		modbus_init(0xAA);
		
		s_last_function_code = 0;
		
		s_read_coils_data.first_coil = UINT16_MAX;
		s_read_coils_data.n_coils = UINT16_MAX;
	}
};

int main()
{
   CppUnit::TextUi::TestRunner runner;
   
   CPPUNIT_TEST_SUITE_REGISTRATION( ModbusTest );

   CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();

   runner.addTest( registry.makeTest() );
   runner.run();

   return 0;
}
