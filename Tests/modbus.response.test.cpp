#include <stdint.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "modbus.h"

static const uint8_t TEST_ADDRESS = 0x65;

class ModbusResponseTest : public CppUnit::TestFixture  {

	CPPUNIT_TEST_SUITE(ModbusResponseTest);

	CPPUNIT_TEST(test_modbus_start_response_writes_address_and_function_code_bytes);
	CPPUNIT_TEST(test_modbus_write_16bit_value);
	CPPUNIT_TEST(test_modbus_write_8bit_value);
	CPPUNIT_TEST(test_modbus_write_discrete_input_values_start1_read_many);
	CPPUNIT_TEST(test_modbus_write_discrete_input_values_start_not_1_read_many);
	CPPUNIT_TEST(test_modbus_write_discrete_input_values_start_not_1_read_one);
	CPPUNIT_TEST(test_modbus_write_discrete_input_values_start_1_read_one);

	CPPUNIT_TEST(test_modbus_write_crc);

	CPPUNIT_TEST_SUITE_END();

	void test_modbus_start_response_writes_address_and_function_code_bytes()
	{
		uint8_t buffer[64];
		int bytes_written = modbus_start_response(buffer, READ_INPUT_REGISTERS, TEST_ADDRESS);

		CPPUNIT_ASSERT_EQUAL(2, bytes_written);
		CPPUNIT_ASSERT_EQUAL(TEST_ADDRESS, buffer[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)READ_INPUT_REGISTERS, buffer[1]);
	}

	void test_modbus_write_16bit_value()
	{
		uint8_t buffer[64];
		int bytes_written = modbus_write(buffer, (int16_t)0x6534);

		CPPUNIT_ASSERT_EQUAL(2, bytes_written);
		CPPUNIT_ASSERT_EQUAL((int)0x65, (int)buffer[0]);
		CPPUNIT_ASSERT_EQUAL((int)0x34, (int)buffer[1]);
	}

	void test_modbus_write_8bit_value()
	{
		uint8_t buffer[64];
		int bytes_written = modbus_write(buffer, (int8_t)0x01);
		bytes_written += modbus_write(buffer+1, (int8_t)0x02);
		bytes_written += modbus_write(buffer+2, (int8_t)0x00);
		bytes_written += modbus_write(buffer+3, (int8_t)0xFF);

		CPPUNIT_ASSERT_EQUAL(4, bytes_written);
		CPPUNIT_ASSERT_EQUAL((int)0x01, (int)buffer[0]);
		CPPUNIT_ASSERT_EQUAL((int)0x02, (int)buffer[1]);
		CPPUNIT_ASSERT_EQUAL((int)0x00, (int)buffer[2]);
		CPPUNIT_ASSERT_EQUAL((int)0xFF, (int)buffer[3]);
	}

	void test_modbus_write_discrete_input_values_start1_read_many()
	{
		bool test_values[] = {false, true, false, true, true, false, true};
		uint8_t expected_read_start1_read7 = 0b01011010;
		uint8_t expected_read_start1_read5 = 0b00011010;

		uint8_t buffer[64];
		int bytes_written = modbus_write_read_discrete_inputs_response(TEST_ADDRESS, buffer, test_values + 0, 7);
		CPPUNIT_ASSERT_EQUAL(6, bytes_written);
		CPPUNIT_ASSERT_EQUAL(TEST_ADDRESS, buffer[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)READ_DISCRETE_INPUTS, buffer[1]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)1, buffer[2]);
		CPPUNIT_ASSERT_EQUAL(expected_read_start1_read7, buffer[3]);

		bytes_written = modbus_write_read_discrete_inputs_response(TEST_ADDRESS, buffer, test_values + 0, 5);
		CPPUNIT_ASSERT_EQUAL(6, bytes_written);
		CPPUNIT_ASSERT_EQUAL(TEST_ADDRESS, buffer[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)READ_DISCRETE_INPUTS, buffer[1]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)1, buffer[2]);
		CPPUNIT_ASSERT_EQUAL(expected_read_start1_read5, buffer[3]);
	}

	void test_modbus_write_discrete_input_values_start_not_1_read_many()
	{
		bool test_values[] = {false, true, false, true, true, false, true};
		uint8_t expected_read_start3_read3 = 0b00000110;
		uint8_t expected_read_start5_read2 = 0b00000001;

		uint8_t buffer[64];
		int bytes_written = modbus_write_read_discrete_inputs_response(TEST_ADDRESS, buffer, test_values + 2, 3);
		CPPUNIT_ASSERT_EQUAL(6, bytes_written);
		CPPUNIT_ASSERT_EQUAL(TEST_ADDRESS, buffer[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)READ_DISCRETE_INPUTS, buffer[1]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)1, buffer[2]);
		CPPUNIT_ASSERT_EQUAL(expected_read_start3_read3, buffer[3]);

		bytes_written = modbus_write_read_discrete_inputs_response(TEST_ADDRESS, buffer, test_values + 4, 2);
		CPPUNIT_ASSERT_EQUAL(TEST_ADDRESS, buffer[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)READ_DISCRETE_INPUTS, buffer[1]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)1, buffer[2]);
		CPPUNIT_ASSERT_EQUAL(expected_read_start5_read2, buffer[3]);
	}

	void test_modbus_write_discrete_input_values_start_not_1_read_one()
	{
		bool test_values[] = {false, true, false, true, true, false, true};
		uint8_t expected_read_start2_read1 = 0b00000001;
		uint8_t expected_read_start6_read1 = 0b00000000;

		uint8_t buffer[64];
		int bytes_written = modbus_write_read_discrete_inputs_response(TEST_ADDRESS, buffer, test_values + 1, 1);
		CPPUNIT_ASSERT_EQUAL(6, bytes_written);
		CPPUNIT_ASSERT_EQUAL(TEST_ADDRESS, buffer[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)READ_DISCRETE_INPUTS, buffer[1]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)1, buffer[2]);
		CPPUNIT_ASSERT_EQUAL(expected_read_start2_read1, buffer[3]);

		bytes_written = modbus_write_read_discrete_inputs_response(TEST_ADDRESS, buffer, test_values + 5, 1);
		CPPUNIT_ASSERT_EQUAL(TEST_ADDRESS, buffer[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)READ_DISCRETE_INPUTS, buffer[1]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)1, buffer[2]);
		CPPUNIT_ASSERT_EQUAL(expected_read_start6_read1, buffer[3]);
	}

	void test_modbus_write_discrete_input_values_start_1_read_one()
	{
		bool test_values[] = {true, false, false, false, false, false, false};
		uint8_t expected_read_start1_read1 = 0b00000001;

		uint8_t buffer[64];
		int bytes_written = modbus_write_read_discrete_inputs_response(TEST_ADDRESS, buffer, test_values, 1);
		CPPUNIT_ASSERT_EQUAL(6, bytes_written);
		CPPUNIT_ASSERT_EQUAL(TEST_ADDRESS, buffer[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)READ_DISCRETE_INPUTS, buffer[1]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)1, buffer[2]);
		CPPUNIT_ASSERT_EQUAL(expected_read_start1_read1, buffer[3]);
	}

	void test_modbus_write_crc()
	{
		uint8_t buffer[] = {0xD4, 0xE3, 0x39, 0x8C, 0x23, 0xA4, 0x00, 0x00};
		
		int bytes_written = 0;
		bytes_written = modbus_write_crc(buffer, 6);

		CPPUNIT_ASSERT_EQUAL(2, bytes_written);
		CPPUNIT_ASSERT_EQUAL((int)0x25, (int)buffer[6]);
		CPPUNIT_ASSERT_EQUAL((int)0x02, (int)buffer[7]);
	}

};


int main()
{
   CppUnit::TextUi::TestRunner runner;
   
   CPPUNIT_TEST_SUITE_REGISTRATION( ModbusResponseTest );

   CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();

   runner.addTest( registry.makeTest() );
   runner.run();

   return 0;
}
