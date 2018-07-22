#ifndef _MPU_H_
#define _MPU_H_

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

#include "mpu_types.h"


#define TO_LE(be) (int16_t)((((be) & 0xFF00) >> 8) | ((be) & 0x00FF) << 8)
#define MPU_ACC_TO_G(acc)   ((float)TO_LE(acc) / 16384.0)
#define MPU_GYR_TO_RAD(gyr) ((float)TO_LE(gyr) / 131.0)
#define MPU_TMP_TO_DEG(tmp) ((float)TO_LE(tmp) / 340.0 + 36.53 )

esp_err_t mpu_open(struct mpu *mpu);
esp_err_t mpu_config(const struct mpu *mpu);
esp_err_t mpu_read_sensors(struct raw_sensors *raw_sensors, struct mpu *mpu);

// Registers
#define MPU_SMPLRT_DIV   0x19
#define MPU_CONFIG       0x1A
#define MPU_GYRO_CONFIG  0x1B
#define MPU_ACCEL_CONFIG 0x1C
#define MPU_INT_ENABLE   0x38
#define MPU_SENSORS      0x3B
#define MPU_ACCEL        0x3B
#define MPU_TEMP         0x41
#define MPU_GYRO         0x43
#define MPU_PWR_MGMT_1   0x6B

// Bits
#define MPU_B_DEVICE_RESET  0x80
#define MPU_B_CLK_SEL_INT   0x00
#define MPU_B_CLK_SEL_GYRX  0x01
#define MPU_B_CLK_SEL_GYRY  0x02
#define MPU_B_CLK_SEL_GYRZ  0x03
#define MPU_B_CLK_SEL_EXT1  0x04
#define MPU_B_CLK_SEL_EXT2  0x05
#define MPU_B_CLK_SEL_STOP  0x07

#endif
