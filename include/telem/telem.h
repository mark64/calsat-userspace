/* userspace sensor helper functions
 *
 * by Mark Hill 2017
 */
#ifndef __sensors_h
#define __sensors_h

/*
 * @return			the unreglated system voltage
 */
double sens_unreg_sys_voltage(void);
/*
 * @return			the unregulated system current
 */
double sens_unreg_sys_current(void);

/*
 * @return			the battery voltage
 */
double sens_batt_voltage(void);
/*
 * @return			the battery current
 */
double sens_batt_current(void);

/*
 * @return			the true voltage for the 3V3 rail
 */
double sens_3v3_voltage(void);
/*
 * @return			the current through 3V3 regulator 0
 */
double sens_3v3_current_0(void);
/*
 * @return			the current through 3V3 regulator 1
 */
double sens_3v3_current_1(void);


#endif
