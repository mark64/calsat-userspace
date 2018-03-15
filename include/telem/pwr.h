/* userspace power sensor helper functions
 *
 * by Mark Hill 2017
 */
#ifndef __pwr_h
#define __pwr_h

#include<drivers/ina3221.h>

/**
 * Here is a list of guarunteed power values
 * there may be others, but behavior is defined for these
 * In case it's not clear, V = voltage, C = current
 */
#define SYS_UNREG_V 	0
#define SYS_UNREG_C 	1
#define BATTERY_V		2
#define BATTERY_C		3
#define SUPPLY_3V3_V	4
#define SUPPLY_3V3_0_C	5 /* there are multiple */
#define SUPPLY_3V3_1_C	7 /* voltage regulators */
#define SOLAR_UNREG_0_V	8
#define SOLAR_UNREG_0_C	9
#define SOLAR_UNREG_1_V	10
#define SOLAR_UNREG_1_C	11

static const int adc_for_attr[] = 		{0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1};
static const int channel_for_attr[] = 	{0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2};

/**
 * gets the value of the specified power attribute
 * @attribute			the number of the attribute
 *
 * @return				a double representation of the
 * 							attribute
 */
inline double pwr_value_for_attribute(int attribute) {
	if (attribute < 12 && attribute % 2 == 0)
		return ina_voltage(adc_for_attr[attribute], channel_for_attr[attribute]);
	else
		return ina_current(adc_for_attr[attribute], channel_for_attr[attribute]);
}

#endif
