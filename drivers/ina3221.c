/**
 * ina3221 driver implemenetation
 *
 * Mark Hill 2017
 */

#include<drivers/ina3221.h>

#include<stdint.h>
#include<drivers/i2cctl.h>

static int __ic_initialized[4];

int ina_init_ic(uint8_t ic) {

}

int ina_sleep_ic(uint8_t ic) {

}

int ina_wake_ic(uint16_t ic) {

}

double ina_voltage(uint8_t ic, uint8_t channel) {

}

double ina_current(uint8_t ic, uint8_t channel) {

}
