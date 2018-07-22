#ifndef _IMU_H_
#define _IMU_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "mpu_types.h"

struct imu {
	struct mpu mpu;
	QueueHandle_t queue;
};

BaseType_t imu_init(struct imu *imu);
void imu_raw2float(struct float_sensors *fs, struct raw_sensors *rs);
void imu_print_sensors(const struct float_sensors *s);

#endif
