#include <stdint.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "modbus.h"

static int s_last_function_code;

static const int MAXIMUM_BUFFER_SIZE = 8;

static uint8_t s_write_multiple_coils_data_buffer[MAXIMUM_BUFFER_SIZE];
static int16_t s_write_holding_register_data_buffer[MAXIMUM_BUFFER_SIZE];

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

static struct _write_multiple_coils_data {uint16_t first_coil; uint16_t n_coils; uint8_t n_values; uint8_t * values;} s_write_multiple_coils_data;
static void write_multiple_coils(uint16_t first_coil, uint16_t n_coils, uint8_t n_values, uint8_t * values)
{
	s_write_multiple_coils_data.first_coil = first_coil;
	s_write_multiple_coils_data.n_coils = n_coils;
	s_write_multiple_coils_data.n_values = n_values;
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

static struct _write_holding_registers_data { uint16_t first_reg; uint16_t n_registers; uint8_t n_values; int16_t * values; } s_write_holding_registers_data;
static void write_holding_registers(uint16_t first_reg, uint16_t n_registers, uint8_t n_values, int16_t * values)
{
	s_write_holding_registers_data.first_reg = first_reg;
	s_write_holding_registers_data.n_registers = n_registers;
	s_write_holding_registers_data.n_values = n_values;
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

static const MODBUS_HANDLER s_modbus_handler = {
	read_coils,
	read_discrete_inputs,
	write_single_coil,
	write_multiple_coils,
	read_input_registers,
	read_holding_registers,
	write_holding_register,
	write_holding_registers,
	read_write_registers,
	mask_write_register,

	s_write_multiple_coils_data_buffer,
	MAXIMUM_BUFFER_SIZE,
	s_write_holding_register_data_buffer,
	MAXIMUM_BUFFER_SIZE
};

class ModbusTest : public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(ModbusTest);

	CPPUNIT_TEST(test_service_with_no_message_does_not_call_any_functions);
	CPPUNIT_TEST(test_service_with_wrong_address_does_not_call_any_functions);
	CPPUNIT_TEST(test_service_with_read_coils_message);
	CPPUNIT_TEST(test_service_with_read_discrete_inputs_message);
	CPPUNIT_TEST(test_service_with_invalid_write_single_coil_message);
	CPPUNIT_TEST(test_service_with_valid_write_single_coil_message);
	CPPUNIT_TEST(test_service_with_write_multiple_coils_message);
	CPPUNIT_TEST(test_service_with_read_input_registers_message);
	CPPUNIT_TEST(test_service_with_read_holding_registers_message);
	CPPUNIT_TEST(test_service_with_write_holding_register_message);
	CPPUNIT_TEST(test_service_with_write_holding_registers_message);
	CPPUNIT_TEST(test_service_with_read_write_registers_message);
	CPPUNIT_TEST(test_service_with_mask_write_register_message);

	CPPUNIT_TEST(test_service_does_not_write_more_than_max_holding_registers);
	CPPUNIT_TEST(test_service_does_not_write_more_than_max_coils);

	CPPUNIT_TEST_SUITE_END();

	void test_service_with_no_message_does_not_call_any_functions()
	{
		char message[] = {(char)0xAB};
		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_wrong_address_does_not_call_any_functions()
	{
		modbus_service_message(NULL, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_read_coils_message()
	{
		char message[] = {(char)0xAA, (char)READ_COILS, (char)0x80, (char)0xFF, (char)0x80, (char)0xF1};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)READ_COILS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80FF, s_read_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80F1, s_read_coils_data.n_coils);
	}

	void test_service_with_read_discrete_inputs_message()
	{
		char message[] = {(char)0xAA, (char)READ_DISCRETE_INPUTS, (char)0x80, (char)0xFF, (char)0x80, (char)0xF1};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)READ_DISCRETE_INPUTS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80FF, s_read_discrete_inputs_data.first_input);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80F1, s_read_discrete_inputs_data.n_inputs);
	}

	void test_service_with_invalid_write_single_coil_message()
	{
		char message[] = {(char)0xAA, (char)WRITE_SINGLE_COIL, (char)0x80, (char)0xFF, (char)0x00, (char)0x01};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_valid_write_single_coil_message()
	{
		char message[] = {(char)0xAA, (char)WRITE_SINGLE_COIL, (char)0x80, (char)0xFF, (char)0xFF, (char)0x00};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_SINGLE_COIL, s_last_function_code);

		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80FF, s_write_single_coil_data.coil);
		CPPUNIT_ASSERT(s_write_single_coil_data.on);
	}

	void test_service_with_write_multiple_coils_message()
	{
		char message[] = {
			(char)0xAA, (char)WRITE_MULTIPLE_COILS,
			(char)0x50, (char)0x43,
			(char)0x00, (char)0x05,
			(char)0x01,
			(char)0b00001011};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_MULTIPLE_COILS, s_last_function_code);

		CPPUNIT_ASSERT_EQUAL((uint16_t)0x5043, s_write_multiple_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0005, s_write_multiple_coils_data.n_coils);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x01, s_write_multiple_coils_data.n_values);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0b00001011, s_write_multiple_coils_data.values[0]);
	}

	void test_service_with_read_input_registers_message()
	{
		char message[] = {(char)0xAA, (char)READ_INPUT_REGISTERS, (char)0x80, (char)0xFF, (char)0x00, (char)0x02};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)READ_INPUT_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80FF, s_read_input_registers_data.reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0002, s_read_input_registers_data.n_registers);
	}

	void test_service_with_read_holding_registers_message()
	{
		char message[] = {(char)0xAA, (char)READ_HOLDING_REGISTERS, (char)0x80, (char)0xFF, (char)0x00, (char)0x04};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)READ_HOLDING_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80FF, s_read_holding_registers_data.reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0004, s_read_holding_registers_data.n_registers);
	}

	void test_service_with_write_holding_register_message()
	{
		char message[] = {(char)0xAA, (char)WRITE_HOLDING_REGISTER, (char)0x80, (char)0xFF, (char)0x01, (char)0x43};
		
		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_HOLDING_REGISTER, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x80FF, s_write_holding_register_data.reg);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0143, s_write_holding_register_data.value);
	}

	void test_service_with_write_holding_registers_message()
	{
		char message[] = {
			(char)0xAA, (char)WRITE_HOLDING_REGISTERS,
			(char)0x50, (char)0x08,
			(char)0x00, (char)0x02,
			(char)0x04,
			(char)0x00, (char)0x50,
			(char)0xF0, (char)0x00,
		};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_HOLDING_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x5008, s_write_holding_registers_data.first_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0002, s_write_holding_registers_data.n_registers);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x04, s_write_holding_registers_data.n_values);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0050, s_write_holding_register_data_buffer[0]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF000, s_write_holding_register_data_buffer[1]);
	}

	void test_service_with_read_write_registers_message()
	{
		char message[] = {
			(char)0xAA, (char)READ_WRITE_REGISTERS,
			(char)0x41, (char)0x08,
			(char)0x00, (char)0x02,
			(char)0x42, (char)0x55,
			(char)0x00, (char)0x06,
			(char)0xF0, (char)0x00, (char)0xF0, (char)0x01, (char)0xF0, (char)0x02,
			(char)0xF0, (char)0x03, (char)0xF0, (char)0x04, (char)0xF0, (char)0x05,
		};

		modbus_service_message(message, s_modbus_handler);
		
		CPPUNIT_ASSERT_EQUAL((int)READ_WRITE_REGISTERS, s_last_function_code);
	
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x4108, s_read_write_registers_data.read_start_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0002, s_read_write_registers_data.n_read_count);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x4255, s_read_write_registers_data.write_start_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0006, s_read_write_registers_data.n_write_count);

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
			(char)0x53, (char)0x38,
			(char)0x03, (char)0xFF,
			(char)0x00, (char)0x7F,
		};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)MASK_WRITE_REGISTER, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x5338, s_mask_write_register_data.reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x03FF, s_mask_write_register_data.and_mask);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x007F, s_mask_write_register_data.or_mask);
	}

	void test_service_does_not_write_more_than_max_holding_registers()
	{
		char message[] = {
			(char)0xAA, (char)WRITE_HOLDING_REGISTERS,
			(char)0x50, (char)0x08,
			(char)0x00, (char)0x10,
			(char)0x20,
			(char)0x00, (char)0x00, (char)0x00, (char)0x01, (char)0x00, (char)0x02, (char)0x00, (char)0x03,
			(char)0x00, (char)0x04, (char)0x00, (char)0x05, (char)0x00, (char)0x06, (char)0x00, (char)0x07,
			(char)0x00, (char)0x08, (char)0x00, (char)0x09, (char)0x00, (char)0x0A, (char)0x00, (char)0x0B,
			(char)0x00, (char)0x0C, (char)0x00, (char)0x0D, (char)0x00, (char)0x0E, (char)0x00, (char)0x0F,
		};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_HOLDING_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x5008, s_write_holding_registers_data.first_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0008, s_write_holding_registers_data.n_registers);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x10, s_write_holding_registers_data.n_values);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0000, s_write_holding_register_data_buffer[0]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0001, s_write_holding_register_data_buffer[1]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0002, s_write_holding_register_data_buffer[2]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0003, s_write_holding_register_data_buffer[3]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0004, s_write_holding_register_data_buffer[4]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0005, s_write_holding_register_data_buffer[5]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0006, s_write_holding_register_data_buffer[6]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0007, s_write_holding_register_data_buffer[7]);
	}

	void test_service_does_not_write_more_than_max_coils()
	{
		char message[] = {
			(char)0xAA, (char)WRITE_MULTIPLE_COILS,
			(char)0x50, (char)0x08,
			(char)0x00, (char)0x80,
			(char)0x10,
			(char)0x00, (char)0x01, (char)0x02, (char)0x03, (char)0x04, (char)0x05, (char)0x06, (char)0x07,
			(char)0x08, (char)0x09, (char)0x0A, (char)0x0B, (char)0x0C, (char)0x0D, (char)0x0E, (char)0x0F,
		};

		modbus_service_message(message, s_modbus_handler);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_MULTIPLE_COILS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x5008, s_write_multiple_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0008, s_write_multiple_coils_data.n_coils);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x01, s_write_multiple_coils_data.n_values);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x00, s_write_multiple_coils_data_buffer[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x01, s_write_multiple_coils_data_buffer[1]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x02, s_write_multiple_coils_data_buffer[2]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x03, s_write_multiple_coils_data_buffer[3]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x04, s_write_multiple_coils_data_buffer[4]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x05, s_write_multiple_coils_data_buffer[5]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x06, s_write_multiple_coils_data_buffer[6]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0x07, s_write_multiple_coils_data_buffer[7]);
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
