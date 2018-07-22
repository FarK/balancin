#include "imu.h"
#include "mpu.h"

static void mpu_task(void *param);

BaseType_t imu_init(struct imu *imu)
{
	BaseType_t err;

	imu->queue = xQueueCreate(1, sizeof(struct raw_sensors));
	if (!imu->queue) {
		printf("Error allocating queue for IMU\n");
		return pdFALSE;
	}

	err =  xTaskCreate(
		mpu_task,
		"mpu_task",
		2*1024,
		(void *)imu,
		4,
		NULL
	);
	if (err != pdTRUE) {
		printf("Error creating IMU task\n");
		vQueueDelete(imu->queue);
	}

	return err;
}

void mpu_task(void *param)
{
	struct raw_sensors sensors;
	struct imu *imu;
	struct mpu *mpu;
	esp_err_t err;

	imu = (struct imu *)param;
	mpu = &imu->mpu;

	do {
		err = mpu_open(mpu);
		vTaskDelay(200 / portTICK_RATE_MS);
	} while (err != ESP_OK);

	do {
		err = mpu_config(mpu);
		vTaskDelay(200 / portTICK_RATE_MS);
	} while (err != ESP_OK);

	while (true) {
		err = mpu_read_sensors(&sensors, mpu);
		if (err != ESP_OK) {
			printf("ERROR %X\n", err);
			mpu_config(mpu);
		} else {
			xQueueSendToFront(imu->queue, (void *)&sensors,
				portMAX_DELAY);
		}

		vTaskDelay(100 / portTICK_RATE_MS);
	}
}

void imu_raw2float(struct float_sensors *fs, struct raw_sensors *rs)
{
	fs->acc[0] =   MPU_ACC_TO_G(rs->acc[0]);
	fs->acc[1] =   MPU_ACC_TO_G(rs->acc[1]);
	fs->acc[2] =   MPU_ACC_TO_G(rs->acc[2]);
	fs->gyr[0] = MPU_GYR_TO_RAD(rs->gyr[0]);
	fs->gyr[1] = MPU_GYR_TO_RAD(rs->gyr[1]);
	fs->gyr[2] = MPU_GYR_TO_RAD(rs->gyr[2]);
	fs->tmp    = MPU_TMP_TO_DEG(rs->tmp);
}

void imu_print_sensors(const struct float_sensors *s)
{
	printf("[%+1.2f] [%+1.2f] [%+1.2f]\t"
		"[%+3.2f] [%+3.2f] [%+3.2f]\t"
		"[%+1.2f]\n",
		s->acc[0],
		s->acc[1],
		s->acc[2],
		s->gyr[0],
		s->gyr[1],
		s->gyr[2],
		s->tmp
	);
}

