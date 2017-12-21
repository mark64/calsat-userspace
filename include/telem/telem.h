/* userspace telemetry helper functions
 *
 * by Mark Hill 2017
 */
#ifndef __telemetry_h
#define __telemetry_h

/**
 * Here is a list of guarunteed attributes
 * there may be others, but behavior is defined for these
 * In case it's not clear, V = voltage, C = current
 */
#define SYS_UNREG_V 	"sys_unreg_v"
#define SYS_UNREG_C 	"sys_unreg_c"
#define BATTERY_V		"batt_v"
#define BATTERY_C		"batt_c"
#define SUPPLY_3V3_V	"3v3_v"
#define SUPPLY_3V3_0_C	"3v3_0_c" /* there are multiple */
#define SUPPLY_3V3_1_C	"3v3_1_c" /* voltage regulators */

#define PWR_TELEM_PATH 	"/sys/class/pmu"

/**
 * gets the value of the specified telemetry attribute
 * @attribute			the name of the attribute
 * 							corresponds to the filename
 * 							in PWR_TELEM_PATH
 *
 * @return				a double representation of the
 * 							attribute file contents
 */
double telem_for_attribute(const char *attribute);

#endif
