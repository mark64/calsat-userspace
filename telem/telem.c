/*
 * implementation for sensor functions
 *
 * by Mark Hill 2017
 */

#include<sensors.h>

#define SYSFS_PATH /sys/class
#define PWR_SENS_PATH SYSFS_PATHpowersupply/

/*
 * internal function for retrieving sysfs values
 * @buf					the buffer in which to copy the value
 * @path				the full path to the sysfs file
 *
 * @return				the number of bytes copied, or that would have
 * 							been copied had buf been large enough
 * 							see strlcpy(3bsd)
 */
ssize_t get_sysfs_value_at_path(char *buf, char *path) {
	
}

double sens_unreg_sys_voltage() {
	
}

