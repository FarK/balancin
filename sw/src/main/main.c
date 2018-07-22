#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/i2c.h"

#include <stdlib.h>
#include <string.h>

#include "imu.h"

static struct imu imu = {
	.mpu = {
		.i2c_num     = I2C_NUM_1,
		.sda_pin     = 18,
		.scl_pin     = 19,
		.drdy_pin    = 21,
		.clk_speed   = 10000,
		.device_addr = 0x68,
	},
	.queue = NULL,
};

void app_main()
{
	struct raw_sensors   raw_sensors;
	struct float_sensors float_sensors;

	if (imu_init(&imu) != pdPASS)
		return;

	while (true) {
		xQueueReceive(imu.queue, &raw_sensors, portMAX_DELAY);
		imu_raw2float(&float_sensors, &raw_sensors);
		imu_print_sensors(&float_sensors);
	}
}
