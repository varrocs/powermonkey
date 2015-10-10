/* 
 Pin Layout:
	 --------
RESET 	1|	|8 VCC
PB3	2|	|7 SCL
PB4	3|	|6 -
GND	4|	|5 SDA
	 --------
*/

extern "C" {
#include "usitwislave/usitwislave.h"
}
#include <avr/io.h>
#include <util/delay.h>
#include "attiny_utils/utils.h"

static const int TWI_ADDRESS=33;

static const int PIN_IDLE = PB3;
static const int PIN_POWER = PB4;

static bool delay_in_action = false;
static uint8_t poweroff_delay	= 0;
static uint16_t reanimate_delay	= 0;

static inline void delay_sec(uint8_t sec) 
{
	while(sec--) _delay_ms(1000);
}

typedef enum power_command {
	POWEROFF_THEN_REANIMATE = 1,
} power_command;

static void parse_command(uint8_t input_buffer_length, const uint8_t* input_buffer)
{
	if (input_buffer_length < 4) 
	{
		return;
	}

	power_command command = (power_command)input_buffer[0];
	switch (command) {
		case POWEROFF_THEN_REANIMATE:
			poweroff_delay = input_buffer[1];
			reanimate_delay = (input_buffer[2] << 8) | (input_buffer[3]);
			break;
		default:
			break;
	};
}

static void on_data(
		uint8_t input_buffer_length, 
		const uint8_t* input_buffer,
		uint8_t *output_buffer_length, 
		uint8_t *output_buffer) {
	parse_command(input_buffer_length, input_buffer);
	return;
}

static void on_idle(void) {
	flash(PIN_IDLE, 1);

	if (delay_in_action) return;

	if (poweroff_delay && reanimate_delay) 
	{
		delay_in_action = true;
		delay_sec(poweroff_delay);
		pin_out_low(PIN_POWER);
		delay_sec(reanimate_delay);
		pin_out_high(PIN_POWER);

		poweroff_delay = 0;
		reanimate_delay = 0; 
		delay_in_action = false;
	}


}

static void setup() {
	pin_mode_out(PIN_IDLE);
	pin_mode_out(PIN_POWER);
}

int main() {
	setup();
	flash(PIN_IDLE, 3);

	pin_out_high(PIN_POWER);

	usi_twi_slave(
			TWI_ADDRESS,
			1, // use sleep
			on_data,
			on_idle
			);
	for(;;) {}
	return 0; 
}
