/**
 * implementation for telemetry functions
 *
 * by Mark Hill 2017
 */

#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>

#include<telem/telem.h>

#define PATH_LEN_MAX 40 /* <-- includes PWR_telem_PATH length */
/**
 * internal function for retrieving sysfs values
 * @buf					the buffer in which to copy the value
 * @buf_len				the size of the buffer
 * @path				the filename of the sysfs attribute
 *
 * @return				on success, the number of bytes copied into buffer
 * 							-1 on IO failure
 */
ssize_t get_sysfs_value_at_path(char *buf, ssize_t buf_len, const char *path) {
	char sysfs_path[PATH_LEN_MAX];
	snprintf(sysfs_path, PATH_LEN_MAX, "%s/%s", PWR_TELEM_PATH, path);
	int file;
	if ((file = open(path, O_RDONLY)))
		return pread(file, buf, buf_len, 0);
	return -1;
}

double telem_for_attribute(const char *attribute) {
	char value_buf[PATH_LEN_MAX];
	if (get_sysfs_value_at_path(value_buf, PATH_LEN_MAX, attribute) < 1) {
		printf("ERROR: failed to get sysfs value from power management unit attribute: %s\n", attribute);
		return 0.0;
	}
	double conv_value = atof(value_buf);
	if (conv_value == 0.0)
		printf("WARNING: str -> double conversion for sysfs attribute %s returned 0.0, a possible error value\n", attribute);
	return conv_value;
}
