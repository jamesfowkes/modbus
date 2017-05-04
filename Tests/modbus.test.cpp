#include <stdint.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "modbus.h"

static int s_last_function_code;

static uint8_t const * s_current_message;
static int s_current_message_length;
static uint8_t s_current_message_address;

static const int NUMBER_OF_COILS = 6;
static const int NUMBER_OF_INPUTS = 4;
static const int NUMBER_OF_INPUT_REGISTERS = 4;
static const int NUMBER_OF_HOLDING_REGISTERS = 6;

static bool s_write_multiple_coils_data_buffer[NUMBER_OF_COILS];
static int16_t s_write_holding_register_data_buffer[NUMBER_OF_HOLDING_REGISTERS+1];

static void store_current_message_data()
{
	s_current_message = modbus_get_current_message();
	s_current_message_length = modbus_get_current_message_length();
	s_current_message_address = modbus_get_current_message_address();
}

static struct _read_coils_data {uint16_t first_coil; uint16_t n_coils;} s_read_coils_data;
static void read_coils(uint16_t first_coil, uint16_t n_coils)
{
	s_read_coils_data.first_coil = first_coil;
	s_read_coils_data.n_coils = n_coils;
	s_last_function_code = READ_COILS;
	store_current_message_data();
}

static struct _read_discrete_inputs_data {uint16_t first_input; uint16_t n_inputs;} s_read_discrete_inputs_data;
static void read_discrete_inputs(uint16_t first_input, uint16_t n_inputs)
{
	s_read_discrete_inputs_data.first_input = first_input;
	s_read_discrete_inputs_data.n_inputs = n_inputs;
	s_last_function_code = READ_DISCRETE_INPUTS;
	store_current_message_data();
}

static struct _write_single_coil_data {uint16_t coil; bool on;} s_write_single_coil_data;
static void write_single_coil(uint16_t coil, bool on)
{
	s_write_single_coil_data.coil = coil;
	s_write_single_coil_data.on = on;
	s_last_function_code = WRITE_SINGLE_COIL;
	store_current_message_data();
}

static struct _write_multiple_coils_data {uint16_t first_coil; uint16_t n_coils; bool * values;} s_write_multiple_coils_data;
static void write_multiple_coils(uint16_t first_coil, uint16_t n_coils, bool * values)
{
	s_write_multiple_coils_data.first_coil = first_coil;
	s_write_multiple_coils_data.n_coils = n_coils;
	s_write_multiple_coils_data.values = values;

	s_last_function_code = WRITE_MULTIPLE_COILS;
	store_current_message_data();
}

static struct _read_input_registers_data {uint16_t reg; uint16_t n_registers;} s_read_input_registers_data;
static void read_input_registers(uint16_t reg, uint16_t n_registers)
{
	s_read_input_registers_data.reg = reg;
	s_read_input_registers_data.n_registers = n_registers;
	s_last_function_code = READ_INPUT_REGISTERS;
	store_current_message_data();
}

static struct _read_holding_registers_data {uint16_t reg; uint16_t n_registers;} s_read_holding_registers_data;
static void read_holding_registers(uint16_t reg, uint16_t n_registers)
{
	s_read_holding_registers_data.reg = reg;
	s_read_holding_registers_data.n_registers = n_registers;
	s_last_function_code = READ_HOLDING_REGISTERS;
	store_current_message_data();
}

static struct _write_holding_register_data {uint16_t reg; int16_t value;} s_write_holding_register_data;
static void write_holding_register(uint16_t reg, int16_t value)
{
	s_write_holding_register_data.reg = reg;
	s_write_holding_register_data.value = value;
	s_last_function_code = WRITE_HOLDING_REGISTER;
	store_current_message_data();
}

static struct _write_holding_registers_data { uint16_t first_reg; uint16_t n_registers; int16_t * values; } s_write_holding_registers_data;
static void write_holding_registers(uint16_t first_reg, uint16_t n_registers, int16_t * values)
{
	s_write_holding_registers_data.first_reg = first_reg;
	s_write_holding_registers_data.n_registers = n_registers;
	s_write_holding_registers_data.values = values;
	s_last_function_code = WRITE_HOLDING_REGISTERS;
	store_current_message_data();
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
	store_current_message_data();
}

static struct _mask_write_register_data { uint16_t reg; uint16_t and_mask; uint16_t or_mask; } s_mask_write_register_data;
static void mask_write_register(uint16_t reg, uint16_t and_mask, uint16_t or_mask)
{
	s_mask_write_register_data.reg = reg;
	s_mask_write_register_data.and_mask = and_mask;
	s_mask_write_register_data.or_mask = or_mask;

	s_last_function_code = MASK_WRITE_REGISTER;
	store_current_message_data();
}

static MODBUS_HANDLER s_modbus_handler;

static bool array_is_zero(int16_t * array, int count)
{
	bool zeroed = true;
	for (int i = 0; i < count; i++)
	{
		zeroed &= (array[i] == 0);
	}
	return zeroed;
}

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

	CPPUNIT_TEST(test_service_get_current_message_functionality);
	CPPUNIT_TEST(test_service_get_message_was_broadcast_functionality);

	CPPUNIT_TEST_SUITE_END();

	void test_service_with_no_message_does_not_call_any_functions()
	{
		uint8_t message[] = {(uint8_t)0xAB};
		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_wrong_address_does_not_call_any_functions()
	{
		modbus_service_message(NULL, s_modbus_handler, 0, false);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_check_crc_enabled_does_not_handle_message_with_invalid_crc()
	{
		uint8_t message[] = {(uint8_t)0xAA, (uint8_t)READ_COILS, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)NUMBER_OF_COILS, (uint8_t)(0x13), (uint8_t)(0xF3)};

		modbus_service_message(message, s_modbus_handler, 8, true);
		CPPUNIT_ASSERT_EQUAL(0, s_last_function_code);
	}

	void test_service_with_check_crc_enabled_handles_message_with_valid_crc()
	{
		uint8_t message[] = {(uint8_t)0xAA, (uint8_t)READ_COILS, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)NUMBER_OF_COILS, (uint8_t)(0xA5), (uint8_t)(0xD3)};

		modbus_service_message(message, s_modbus_handler, 8, true);
		CPPUNIT_ASSERT_EQUAL((int)READ_COILS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0000, s_read_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_COILS, s_read_coils_data.n_coils);
	}

	void test_service_with_read_coils_message()
	{
		uint8_t message[] = {(uint8_t)0xAA, (uint8_t)READ_COILS, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)NUMBER_OF_COILS};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)READ_COILS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0000, s_read_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_COILS, s_read_coils_data.n_coils);
	}

	void test_service_with_read_discrete_inputs_message()
	{
		uint8_t message[] = {(uint8_t)0xAA, (uint8_t)READ_DISCRETE_INPUTS, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)NUMBER_OF_INPUTS};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)READ_DISCRETE_INPUTS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0000, s_read_discrete_inputs_data.first_input);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_INPUTS, s_read_discrete_inputs_data.n_inputs);
	}

	void test_service_with_valid_write_single_coil_message()
	{
		uint8_t message[] = {(uint8_t)0xAA, (uint8_t)WRITE_SINGLE_COIL, (uint8_t)0x00, (uint8_t)0x04, (uint8_t)0xFF, (uint8_t)0x00};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_SINGLE_COIL, s_last_function_code);

		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0004, s_write_single_coil_data.coil);
		CPPUNIT_ASSERT(s_write_single_coil_data.on);
	}

	void test_service_with_write_multiple_coils_message()
	{
		uint8_t message[] = {
			(uint8_t)0xAA, (uint8_t)WRITE_MULTIPLE_COILS,
			(uint8_t)0x00, (uint8_t)0x01,
			(uint8_t)0x00, (uint8_t)0x03,
			(uint8_t)0x01,
			(uint8_t)0b00000101};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_MULTIPLE_COILS, s_last_function_code);

		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0001, s_write_multiple_coils_data.first_coil);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0003, s_write_multiple_coils_data.n_coils);
		CPPUNIT_ASSERT_EQUAL(true, s_write_multiple_coils_data.values[0]);
		CPPUNIT_ASSERT_EQUAL(false, s_write_multiple_coils_data.values[1]);
		CPPUNIT_ASSERT_EQUAL(true, s_write_multiple_coils_data.values[2]);
	}

	void test_service_with_read_input_registers_message()
	{
		uint8_t message[] = {(uint8_t)0xAA, (uint8_t)READ_INPUT_REGISTERS, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)NUMBER_OF_INPUT_REGISTERS};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)READ_INPUT_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0000, s_read_input_registers_data.reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_INPUT_REGISTERS, s_read_input_registers_data.n_registers);
	}

	void test_service_with_read_holding_registers_message()
	{
		uint8_t message[] = {(uint8_t)0xAA, (uint8_t)READ_HOLDING_REGISTERS, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)0x00, (uint8_t)NUMBER_OF_HOLDING_REGISTERS};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)READ_HOLDING_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0000, s_read_holding_registers_data.reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_HOLDING_REGISTERS, s_read_holding_registers_data.n_registers);
	}

	void test_service_with_write_holding_register_message()
	{
		uint8_t message[] = {(uint8_t)0xAA, (uint8_t)WRITE_HOLDING_REGISTER, (uint8_t)0x00, (uint8_t)NUMBER_OF_HOLDING_REGISTERS-1, (uint8_t)0x01, (uint8_t)0x43};
		
		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_HOLDING_REGISTER, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)(NUMBER_OF_HOLDING_REGISTERS-1), s_write_holding_register_data.reg);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0143, s_write_holding_register_data.value);
	}

	void test_service_with_write_holding_registers_message_full_addr_range()
	{
		uint8_t message[] = {
			(uint8_t)0xAA, (uint8_t)WRITE_HOLDING_REGISTERS,
			(uint8_t)0x00, (uint8_t)0x00,
			(uint8_t)0x00, (uint8_t)NUMBER_OF_HOLDING_REGISTERS,
			(uint8_t)(NUMBER_OF_HOLDING_REGISTERS * 2),
			(uint8_t)0x00, (uint8_t)0x50,
			(uint8_t)0xF0, (uint8_t)0x22,
			(uint8_t)0xBB, (uint8_t)0x57,
			(uint8_t)0xDF, (uint8_t)0x49,
			(uint8_t)0xE5, (uint8_t)0x33,
			(uint8_t)0x82, (uint8_t)0xC3,
		};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_HOLDING_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0000, s_write_holding_registers_data.first_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_HOLDING_REGISTERS, s_write_holding_registers_data.n_registers);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0050, s_write_holding_register_data_buffer[0]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF022, s_write_holding_register_data_buffer[1]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xBB57, s_write_holding_register_data_buffer[2]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xDF49, s_write_holding_register_data_buffer[3]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xE533, s_write_holding_register_data_buffer[4]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x82C3, s_write_holding_register_data_buffer[5]);

		CPPUNIT_ASSERT(array_is_zero(&s_write_holding_register_data_buffer[6], 1));
	}

	void test_service_with_write_holding_registers_message_last_addr()
	{
		uint8_t message[] = {
			(uint8_t)0xAA, (uint8_t)WRITE_HOLDING_REGISTERS,
			(uint8_t)0x00, (uint8_t)NUMBER_OF_HOLDING_REGISTERS-1,
			(uint8_t)0x00, (uint8_t)0x01,
			(uint8_t)0x02,
			(uint8_t)0x00, (uint8_t)0x50,
		};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)WRITE_HOLDING_REGISTERS, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)(NUMBER_OF_HOLDING_REGISTERS-1), s_write_holding_registers_data.first_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x01, s_write_holding_registers_data.n_registers);
		CPPUNIT_ASSERT_EQUAL((int16_t)0x0050, s_write_holding_register_data_buffer[0]);

		CPPUNIT_ASSERT(array_is_zero(&s_write_holding_register_data_buffer[1], 6));
	}

	void test_service_with_read_write_registers_message()
	{
		uint8_t message[] = {
			(uint8_t)0xAA, (uint8_t)READ_WRITE_REGISTERS,
			(uint8_t)0x00, (uint8_t)0x00,
			(uint8_t)0x00, (uint8_t)NUMBER_OF_HOLDING_REGISTERS,
			(uint8_t)0x00, (uint8_t)0x00,
			(uint8_t)0x00, (uint8_t)NUMBER_OF_HOLDING_REGISTERS,
			(uint8_t)(NUMBER_OF_HOLDING_REGISTERS*2),
			(uint8_t)0xF0, (uint8_t)0x00,
			(uint8_t)0xF0, (uint8_t)0x01,
			(uint8_t)0xF0, (uint8_t)0x02,
			(uint8_t)0xF0, (uint8_t)0x03,
			(uint8_t)0xF0, (uint8_t)0x04,
			(uint8_t)0xF0, (uint8_t)0x05,
		};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		
		CPPUNIT_ASSERT_EQUAL((int)READ_WRITE_REGISTERS, s_last_function_code);
	
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0000, s_read_write_registers_data.read_start_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_HOLDING_REGISTERS, s_read_write_registers_data.n_read_count);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x0000, s_read_write_registers_data.write_start_reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)NUMBER_OF_HOLDING_REGISTERS, s_read_write_registers_data.n_write_count);

		CPPUNIT_ASSERT_EQUAL((int16_t)0xF000, s_write_holding_register_data_buffer[0]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF001, s_write_holding_register_data_buffer[1]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF002, s_write_holding_register_data_buffer[2]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF003, s_write_holding_register_data_buffer[3]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF004, s_write_holding_register_data_buffer[4]);
		CPPUNIT_ASSERT_EQUAL((int16_t)0xF005, s_write_holding_register_data_buffer[5]);

		CPPUNIT_ASSERT(array_is_zero(&s_write_holding_register_data_buffer[6], 1));
	}

	void test_service_with_mask_write_register_message()
	{
		uint8_t message[] = {
			(uint8_t)0xAA, (uint8_t)MASK_WRITE_REGISTER,
			(uint8_t)0x00, (uint8_t)(NUMBER_OF_HOLDING_REGISTERS-1),
			(uint8_t)0x03, (uint8_t)0xFF,
			(uint8_t)0x00, (uint8_t)0x7F,
		};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);
		CPPUNIT_ASSERT_EQUAL((int)MASK_WRITE_REGISTER, s_last_function_code);
		CPPUNIT_ASSERT_EQUAL((uint16_t)(NUMBER_OF_HOLDING_REGISTERS-1), s_mask_write_register_data.reg);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x03FF, s_mask_write_register_data.and_mask);
		CPPUNIT_ASSERT_EQUAL((uint16_t)0x007F, s_mask_write_register_data.or_mask);
	}

	void test_service_get_current_message_functionality()
	{
		uint8_t message[] = {
			(uint8_t)0xAA, (uint8_t)MASK_WRITE_REGISTER,
			(uint8_t)0x00, (uint8_t)(NUMBER_OF_HOLDING_REGISTERS-1),
			(uint8_t)0x03, (uint8_t)0xFF,
			(uint8_t)0x00, (uint8_t)0x7F,
		};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);

		CPPUNIT_ASSERT_EQUAL((uint8_t const * )message, s_current_message);
		CPPUNIT_ASSERT_EQUAL(8, s_current_message_length);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0xAA, s_current_message_address);
	}

	void test_service_get_message_was_broadcast_functionality()
	{
		uint8_t message[] = {
			(uint8_t)0xAA, (uint8_t)MASK_WRITE_REGISTER,
			(uint8_t)0x00, (uint8_t)(NUMBER_OF_HOLDING_REGISTERS-1),
			(uint8_t)0x03, (uint8_t)0xFF,
			(uint8_t)0x00, (uint8_t)0x7F,
		};

		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);

		CPPUNIT_ASSERT(!modbus_last_message_was_broadcast());

		message[0] = MODBUS_BROADCAST_ADDRESS;
		modbus_service_message(message, s_modbus_handler, sizeof(message)/sizeof(uint8_t), false);

		CPPUNIT_ASSERT(modbus_last_message_was_broadcast());
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

		for (int i=0; i<NUMBER_OF_HOLDING_REGISTERS+1; i++)
		{
			s_write_holding_register_data_buffer[i] = 0x0000;
		}

		s_current_message = NULL;
		s_current_message_address = 0x00;
		s_current_message_length = 0;
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
