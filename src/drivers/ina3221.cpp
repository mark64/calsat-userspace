/**
 * ina3221 driver implemenetation
 * see: http://www.ti.com/lit/ds/symlink/ina3221.pdf
 *
 * Mark Hill 2017-2018
 */

#include<drivers/ina3221.h>

#include<stdint.h>
#include<unistd.h>
#include<drivers/i2cctl.h>

enum ic_state {
    OFF,
    ASLEEP,
    ON,
    ERR
};
static enum ic_state __ic_state[4] = {OFF, OFF, OFF, OFF};
static const uint16_t on_config = 0x7723; // 0b0111011100100011
static const uint16_t sleep_config = 0x7720; // 0b0111011100100000
static const uint16_t reset_config = 0x8720; // 0b1111011100100000
static const uint8_t ina_addr[4] = {0x63, 0x64, 0x65, 0x66};

int ina_init_ic(uint8_t ic) {
    if (__ic_state[ic] == ON)
        return 0;
    uint8_t data[3] = {0, on_config >> 8, on_config & 0xff};
    uint8_t addr = ina_addr[ic];
    int ret = 0;
    if ((ret = i2c_write(addr, data, sizeof(data))) == 0)
        __ic_state[ic] = ON;
    else
        __ic_state[ic] = ERR;
    return ret;
}

int ina_sleep_ic(uint8_t ic) {
    if (__ic_state[ic] != ON)
        return -1;
    uint8_t data[3] = {0, sleep_config >> 8, sleep_config & 0xff};
    uint8_t addr = ina_addr[ic];
    int ret = 0;
    if ((ret = i2c_write(addr, data, sizeof(data))) == 0)
        __ic_state[ic] = ASLEEP;
    else
        __ic_state[ic] = ERR;
    return ret;
}

int ina_wake_ic(uint8_t ic) {
    if (__ic_state[ic] != ASLEEP)
        return -1;
    uint8_t data[3] = {0, sleep_config >> 8, sleep_config & 0xff};
    uint8_t addr = ina_addr[ic];
    int ret = 0;
    if ((ret = i2c_write(addr, data, sizeof(data))) == 0)
        __ic_state[ic] = ON;
    else
        __ic_state[ic] = ERR;
    return ret;
}

int ina_reset_ic(uint8_t ic) {
    if (__ic_state[ic] != ASLEEP)
        return -1;
    uint8_t data[3] = {0, reset_config >> 8, reset_config & 0xff};
    uint8_t addr = ina_addr[ic];
    int ret = 0;
    if ((ret = i2c_write(addr, data, sizeof(data))) == 0) {
        __ic_state[ic] = OFF;
        usleep(500);
        return ina_init_ic(ic);
    } else {
        __ic_state[ic] = ERR;
    }
    return ret;
}

double ina_voltage(uint8_t ic, uint8_t channel) {
    ina_init_ic(ic);
    if (__ic_state[ic] != ON)
        return -1;
    uint8_t data[2];
    uint8_t addr = ina_addr[ic];
    int ret = 0;
    if ((ret = i2c_read(addr, 2 * channel + 2, data, sizeof(data))) == 0) {
        int8_t voltage_data_upper = data[1];
        double voltage = ((double)voltage_data_upper) * (1 << 5) + (data[0] >> 3);
        return voltage * 0.008;
    } else {
        __ic_state[ic] = ERR;
    }
    return 0;
}

double ina_current(uint8_t ic, uint8_t channel) {
    ina_init_ic(ic);
    if (__ic_state[ic] != ON)
        return -1;
    uint8_t data[2];
    uint8_t addr = ina_addr[ic];
    int ret = 0;
    if ((ret = i2c_read(addr, 2 * channel + 2, data, sizeof(data))) == 0) {
        int8_t current_data_upper = data[1];
        double current = ((double)current_data_upper) * (1 << 5) + (data[0] >> 3);
        return current * 0.00004;
    } else {
        __ic_state[ic] = ERR;
    }
    return 0;
}
