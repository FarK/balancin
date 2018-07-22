#ifndef _MPU_TYPES_H_
#define _MPU_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct mpu {
	int     i2c_num;
	short   sda_pin;
	short   scl_pin;
	short   drdy_pin;
	int     clk_speed;
	uint8_t device_addr;
	volatile bool drdy;
	SemaphoreHandle_t drdy_semph;
};

struct raw_sensors {
	int16_t acc[3];
	int16_t tmp;
	int16_t gyr[3];
}__attribute__((packed));

struct float_sensors {
	float acc[3];
	float gyr[3];
	float tmp;
};

#endif
