#include <stdint.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "modbus.h"

static int s_last_function_code;

static const int NUMBER_OF_COILS = 6;
static const int NUMBER_OF_INPUTS = 4;
static const int NUMBER_OF_INPUT_REGISTERS = 4;
static const int NUMBER_OF_HOLDING_REGISTERS = 6;

static bool s_write_multiple_coils_data_buffer[NUMBER_OF_COILS];
static int16_t s_write_holding_register_data_buffer[NUMBER_OF_HOLDING_REGISTERS];

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

static struct _write_multiple_coils_data {uint16_t first_coil; uint16_t n_coils; bool * values;} s_write_multiple_coils_data;
static void write_multiple_coils(uint16_t first_coil, uint16_t n_coils, bool * values)
{
	s_write_multiple_coils_data.first_coil = first_coil;
	s_write_multiple_coils_data.n_coils = n_coils;
	s_write_multiple_coils_data.values = values;

	s_last_function_code = WRITE_MULTIPLE_COILS;
}

static struct _read_input_registers_data {uint16_t reg; uint16_t n_registers;} s_read_input_registers_data;
static void read_input_registers(uint16_t reg, uint16_t n_registers)
{
	s_read_input_registers_data.reg = reg;
	s_read_input_registers_data.n_registers = n_registers;
	s_last_function_code = READ_INPUT_REGISTERS;
}

static struct _read_holding_registers_data {uint16_t reg; uint16_t n_registers;} s_read_holding_registers_data;
static void read_holding_registers(uint16_t reg, uint16_t n_registers)
{
	s_read_holding_registers_data.reg = reg;
	s_read_holding_registers_data.n_registers = n_registers;
	s_last_function_code = READ_HOLDING_REGISTERS;
}

static struct _write_holding_register_data {uint16_t reg; int16_t value;} s_write_holding_register_data;
static void write_holding_register(uint16_t reg, int16_t value)
{
	s_write_holding_register_data.reg = reg;
	s_write_holding_register_data.value = value;
	s_last_function_code = WRITE_HOLDING_REGISTER;
}

static struct _write_holding_registers_data { uint16_t first_reg; uint16_t n_registers; int16_t * values; } s_write_holding_registers_data;
static void write_holding_registers(uint16_t first_reg, uint16_t n_registers, int16_t * values)
{
	s_write_holding_registers_data.first_reg = first_reg;
	s_write_holding_registers_data.n_registers = n_registers;
	s_write_holding_registers_data.values = values;
	s_last_function_code = WRITE_HOLDING_REGISTERS;
}

static struct _read_write_registers_data { uint16_t read_start_reg; uint16_t n_read_count; uint16_t write_start_reg; uint16_t n_write_count; int16_t * values; } s_read_write_registers_data;
static void read_write_registers(uint16_t read_start_reg, uint16_t n_read_count, uint16_t write_start_reg, uint16_t n_write_count, int16_t * values)
{
	s_read_write_registers_data.read_start_reg = read_start_reg;
	s_read_write_registers_data.n_read_count = n_read_count;
	s_read_write_registers_data.write_start_reg = write_start_reg;
	s_read_write_registers_data.n_write_count = n_write_count;
	s_read_write_registers_data.values = values;
	s_last_function_code = READ_WRITE_REGISTERS;
}

static struct _mask_write_register_data { uint16_t reg; uint16_t and_mask; uint16_t or_mask; } s_mask_write_register_data;
static void mask_write_register(uint16_t reg, uint16_t and_mask, uint16_t or_mask)
{
	s_mask_write_register_data.reg = reg;
	s_mask_write_register_data.and_mask = and_mask;
	s_mask_write_register_data.or_mask = or_mask;

	s_last_function_code = MASK_WRITE_REGISTER;
}

static MODBUS_HANDLER s_modbus_handler;

class ModbusTest : public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(ModbusTest);

	CPPUNIT_TEST(test_service_with_no_message_does_not_call_any_functions);
	CPPUNIT_TEST(test_service_with_wrong_address_does_not_call_any_functions);
	CPPUNIT_TEST(test_service_with_check_crc_enabled_does_not_handle_message_with_invalid_crc);
	CPPUNIT_TEST(test_service_with_check_crc_enabled_handles_message_with_valid_crc);

	CPPUNIT_TEST(test_service_with_read_coils_message);
	CPPUNIT_TEST(test_service_with_read_discrete_inputs_message);
	CPPUNIT_TEST(test_service_with_valid_write_single_coil_message);
	CPPUNIT_TEST(test_service_with_write_multiple_coils_message);
	CPPUNIT_TEST(test_service_with_read_input_registers_message);
	CPPUNIT_TEST(test_service_with_read_holding_registers_message);
	CPPUNIT_TEST(test_service_with_write_holding_register_message);
	
	CPPUNIT_TEST(test_service_with_write_holding_registers_message_full_addr_range);
	CPPUNIT_TEST(test_service_with_write_holding_registers_message_last_addr);
	
	CPPUNIT_TEST(test_service_with_read_write_registers_message);
	CPPUNIT_TEST(test_service_with_mask_write_register_message);

	CPPUNIT_TEST_SUITE_END();

	void test_service_with_no_message_does_not_call_any_functions()
	{
		char message[] = {(char)0xAB};
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_wrong_address_does_not_call_any_functions()
	{
		modbus_service_message(NULL, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_check_crc_enabled_does_not_handle_message_with_invalid_crc()
	{
		char message[] = {(char)0xAA, (char)READ_COILS, (char)0x00, (char)0x01, (char)0x00, (char)NUMBER_OF_COILS, (char)(0x13), (char)(0xF3)};

		modbus_service_message(message, s_modbus_handler, 8, true);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_check_crc_enabled_handles_message_with_valid_crc()
	{
		char message[] = {(char)0xAA, (char)READ_COILS, (char)0x00, (char)0x01, (char)0x00, (char)NUMBER_OF_COILS, (char)(0x13), (char)(0xF4)};

		modbus_service_message(message, s_modbus_handler, 8, true);
		CPPUNIT_ASSERT_EQUAL((int)READ_COILS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_read_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_COILS, s_read_coils_data.n_coils);
	}

	void test_service_with_read_coils_message()
	{
		char message[] = {(char)0xAA, (char)READ_COILS, (char)0x00, (char)0x01, (char)0x00, (char)NUMBER_OF_COILS};

		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)READ_COILS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_read_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_COILS, s_read_coils_data.n_coils);
	}

	void test_service_with_read_discrete_inputs_message()
	{
		char message[] = {(char)0xAA, (char)READ_DISCRETE_INPUTS, (char)0x00, (char)0x01, (char)0x00, (char)NUMBER_OF_INPUTS};

		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)READ_DISCRETE_INPUTS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_read_discrete_inputs_data.first_input);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_INPUTS, s_read_discrete_inputs_data.n_inputs);
	}

	void test_service_with_valid_write_single_coil_message()
	{
		char message[] = {(char)0xAA, (char)WRITE_SINGLE_COIL, (char)0x00, (char)0x04, (char)0xFF, (char)0x00};

		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_SINGLE_COIL, s_last_function_code);

		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0004, s_write_single_coil_data.coil);
		CPPUNIT_ASSERT(s_write_single_coil_data.on);
	}

	void test_service_with_write_multiple_coils_message()
	{
		char message[] = {
			(char)0xAA, (char)WRITE_MULTIPLE_COILS,
			(char)0x00, (char)0x01,
			(char)0x00, (char)0x03,
			(char)0x01,
			(char)0b00000101};

		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_MULTIPLE_COILS, s_last_function_code);

		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_write_multiple_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0003, s_write_multiple_coils_data.n_coils);
		CPPUNIT_ASSERT_EQUAL(true, s_write_multiple_coils_data.values[0]);
		CPPUNIT_ASSERT_EQUAL(false, s_write_multiple_coils_data.values[1]);
		CPPUNIT_ASSERT_EQUAL(true, s_write_multiple_coils_data.values[2]);
	}

	void test_service_with_read_input_registers_message()
	{
		char message[] = {(char)0xAA, (char)READ_INPUT_REGISTERS, (char)0x00, (char)0x01, (char)0x00, (char)NUMBER_OF_INPUT_REGISTERS};

		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)READ_INPUT_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_read_input_registers_data.reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_INPUT_REGISTERS, s_read_input_registers_data.n_registers);
	}

	void test_service_with_read_holding_registers_message()
	{
		char message[] = {(char)0xAA, (char)READ_HOLDING_REGISTERS, (char)0x00, (char)0x01, (char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS};

		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)READ_HOLDING_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_read_holding_registers_data.reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_HOLDING_REGISTERS, s_read_holding_registers_data.n_registers);
	}

	void test_service_with_write_holding_register_message()
	{
		char message[] = {(char)0xAA, (char)WRITE_HOLDING_REGISTER, (char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS-1, (char)0x01, (char)0x43};
		
		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_HOLDING_REGISTER, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)(NUMBER_OF_HOLDING_REGISTERS-1), s_write_holding_register_data.reg);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0143, s_write_holding_register_data.value);
	}

	void test_service_with_write_holding_registers_message_full_addr_range()
	{
		char message[] = {
			(char)0xAA, (char)WRITE_HOLDING_REGISTERS,
			(char)0x00, (char)0x01,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS,
			(char)(NUMBER_OF_HOLDING_REGISTERS * 2),
			(char)0x00, (char)0x50,
			(char)0xF0, (char)0x22,
			(char)0xBB, (char)0x57,
			(char)0xDF, (char)0x49,
			(char)0xE5, (char)0x33,
			(char)0x82, (char)0xC3,
		};

		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_HOLDING_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_write_holding_registers_data.first_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_HOLDING_REGISTERS, s_write_holding_registers_data.n_registers);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0050, s_write_holding_register_data_buffer[0]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF022, s_write_holding_register_data_buffer[1]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xBB57, s_write_holding_register_data_buffer[2]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xDF49, s_write_holding_register_data_buffer[3]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xE533, s_write_holding_register_data_buffer[4]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x82C3, s_write_holding_register_data_buffer[5]);
	}

	void test_service_with_write_holding_registers_message_last_addr()
	{
		char message[] = {
			(char)0xAA, (char)WRITE_HOLDING_REGISTERS,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS,
			(char)0x00, (char)0x01,
			(char)0x02,
			(char)0x00, (char)0x50,
		};

		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_HOLDING_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_HOLDING_REGISTERS, s_write_holding_registers_data.first_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x01, s_write_holding_registers_data.n_registers);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0050, s_write_holding_register_data_buffer[0]);
	}

	void test_service_with_read_write_registers_message()
	{
		char message[] = {
			(char)0xAA, (char)READ_WRITE_REGISTERS,
			(char)0x00, (char)0x01,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS,
			(char)0x00, (char)0x01,
			(char)0x00, (char)NUMBER_OF_HOLDING_REGISTERS,
			(char)(NUMBER_OF_HOLDING_REGISTERS*2),
			(char)0xF0, (char)0x00,
			(char)0xF0, (char)0x01,
			(char)0xF0, (char)0x02,
			(char)0xF0, (char)0x03,
			(char)0xF0, (char)0x04,
			(char)0xF0, (char)0x05,
		};

		modbus_service_message(message, s_modbus_handler, 0, false);
		
		CPPUNIT_ASSERT_EQUAL((int)READ_WRITE_REGISTERS, s_last_function_code);
	
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_read_write_registers_data.read_start_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_HOLDING_REGISTERS, s_read_write_registers_data.n_read_count);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_read_write_registers_data.write_start_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_HOLDING_REGISTERS, s_read_write_registers_data.n_write_count);

		CPPUNIT_ASSERT_EQUAL((int16_t)0xF000, s_write_holding_register_data_buffer[0]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF001, s_write_holding_register_data_buffer[1]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF002, s_write_holding_register_data_buffer[2]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF003, s_write_holding_register_data_buffer[3]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF004, s_write_holding_register_data_buffer[4]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF005, s_write_holding_register_data_buffer[5]);
	}

	void test_service_with_mask_write_register_message()
	{
		char message[] = {
			(char)0xAA, (char)MASK_WRITE_REGISTER,
			(char)0x00, (char)(NUMBER_OF_HOLDING_REGISTERS-1),
			(char)0x03, (char)0xFF,
			(char)0x00, (char)0x7F,
		};

		modbus_service_message(message, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL((int)MASK_WRITE_REGISTER, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)(NUMBER_OF_HOLDING_REGISTERS-1), s_mask_write_register_data.reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x03FF, s_mask_write_register_data.and_mask);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x007F, s_mask_write_register_data.or_mask);
	}

public:
	void setUp()
	{
		s_modbus_handler.functions.read_coils = read_coils;
		s_modbus_handler.functions.read_discrete_inputs = read_discrete_inputs;
		s_modbus_handler.functions.write_single_coil = write_single_coil;
		s_modbus_handler.functions.write_multiple_coils = write_multiple_coils;
		s_modbus_handler.functions.read_input_registers = read_input_registers;
		s_modbus_handler.functions.read_holding_registers = read_holding_registers;
		s_modbus_handler.functions.write_holding_register = write_holding_register;
		s_modbus_handler.functions.write_holding_registers = write_holding_registers;
		s_modbus_handler.functions.read_write_registers = read_write_registers;
		s_modbus_handler.functions.mask_write_register = mask_write_register;
		s_modbus_handler.functions.exception_handler = NULL;
		s_modbus_handler.data.device_address = 0xAA;
		s_modbus_handler.data.write_multiple_coils = s_write_multiple_coils_data_buffer; 
		s_modbus_handler.data.write_holding_registers = s_write_holding_register_data_buffer; 
		s_modbus_handler.data.num_coils = NUMBER_OF_COILS;
		s_modbus_handler.data.num_inputs =  NUMBER_OF_INPUTS;
		s_modbus_handler.data.num_input_registers = NUMBER_OF_INPUT_REGISTERS;
		s_modbus_handler.data.num_holding_registers = NUMBER_OF_HOLDING_REGISTERS;

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
