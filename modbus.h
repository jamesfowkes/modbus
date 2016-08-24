struct modbus_handler_functions
{
	void (*read_coils)(uint16_t first_coil, uint16_t n_coils);
	void (*read_discrete_inputs)(uint16_t first_input, uint16_t n_inputs);
	void (*write_single_coil)(uint16_t coil, bool on);
	void (*write_multiple_coils)(uint16_t first_coil, uint16_t n_coils, uint8_t n_values, uint8_t * values);
	void (*read_input_registers)(uint16_t reg, uint16_t n_registers);
	void (*read_holding_registers)(uint16_t reg, uint16_t n_registers);
	void (*write_holding_register)(uint16_t reg, uint16_t value);
	void (*write_holding_registers)(uint16_t first_reg, uint16_t n_registers, uint8_t n_values, uint16_t * values);
	void (*read_write_registers)(uint16_t read_start_reg, uint16_t n_registers, uint16_t write_start_reg, uint16_t n_values, uint16_t * values);
	void (*mask_write_register)(uint16_t reg, uint16_t and_mask, uint16_t or_mask);
}
