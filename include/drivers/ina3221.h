/**
 * driver for ina3221 current sense amplifier
 *
 * Mark Hill 2017
 */
#ifndef __ina3221_h
#define __ina3221_h

#include<stdint.h>

#define SHUNT_RESISTANCE 0.01
#define INA_0_ADDR 0x80
#define INA_1_ADDR 0x82

/**
 * puts the given ic into poweroff mode
 * @ic          a number >= 0 referring to which ina3221 ic to use
 *
 * @return      0 on success, -1 on error
 */
int ina_sleep_ic(uint8_t ic);

/**
 * obtains the bus voltage for the given adc and channel
 * will initialize the ic if it is off, unconfigured, or asleep
 * @ic			a number >= 0 referring to which ina3221 ic to use
 * @channel		the channel of the given ic to measure
 *
 * @return		a double representing the bus voltage of the
 * 					channel in V
 */
double ina_voltage(uint8_t ic, uint8_t channel);

/**
 * obtains the bus current for the given adc and channel
 * will initialize the ic if it is off, unconfigured, or asleep
 * @ic			a number >= 0 referring to which ina3221 ic to use
 * @channel		the channel of the given ic to measure
 *
 * @return		a double representing the current of the
 * 					channel in mA
 */
double ina_current(uint8_t ic, uint8_t channel);

#endif
