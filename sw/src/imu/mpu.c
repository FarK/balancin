#include "mpu.h"
#include <stdint.h>
#include <driver/i2c.h>
#include <driver/gpio.h>

#define ACK_CHECK  0x1  /*!< I2C master will check ack from slave*/
#define ACK        0x0  /*!< I2C ack value */
#define NACK       0x1  /*!< I2C nack value */

static esp_err_t rx(char *data, char reg_addr, size_t size,
	const struct mpu *mpu);
static esp_err_t tx(char reg_addr, char *data, size_t size,
	const struct mpu *mpu);
static esp_err_t write_reg(char addr, char val, const struct mpu *mpu);
static esp_err_t read_reg(char *val, char addr, const struct mpu *mpu);
static void IRAM_ATTR drdy_isr(void *param);

esp_err_t mpu_open(struct mpu *mpu)
{
	const gpio_config_t gpio_conf = {
		.pin_bit_mask = (1 << mpu->drdy_pin),
		.mode         = GPIO_MODE_INPUT,
		.pull_up_en   = 0,
		.pull_down_en = 0,
		.intr_type    = GPIO_INTR_ANYEDGE,
	};

	const i2c_config_t conf = {
		.mode             = I2C_MODE_MASTER,
		.sda_io_num       = mpu->sda_pin,
		.scl_io_num       = mpu->scl_pin,
		.sda_pullup_en    = GPIO_PULLUP_ENABLE,
		.scl_pullup_en    = GPIO_PULLUP_ENABLE,
		.master.clk_speed = mpu->clk_speed,
	};
	esp_err_t err;

	err = i2c_param_config(mpu->i2c_num, &conf);
	if (err != ESP_OK) {
		printf("Error configuring i2c parameters\n");
		return err;
	}

	err = i2c_driver_install(mpu->i2c_num, conf.mode, 0, 0, 0);
	if (err != ESP_OK) {
		printf("Error installing i2c driver\n");
		return err;
	}

	err = gpio_config(&gpio_conf);
	if (err != ESP_OK) {
		printf("Error configuring GPIO\n");
		return err;
	}

	mpu->drdy_semph = xSemaphoreCreateBinary();
	if (!mpu->drdy_semph) {
		printf("Error creating DRDY semaphore\n");
		return -1;
	}

	err = gpio_install_isr_service(0);
	printf("DONE\n");
	if (err != ESP_OK) {
		printf("Error installing GPIO ISR service\n");
		return err;
	}

	err = gpio_isr_handler_add(mpu->drdy_pin, drdy_isr, mpu);
	if (err != ESP_OK) {
		printf("Error adding GPIO ISR\n");
		return err;
	}

	return err;
}

static void IRAM_ATTR drdy_isr(void *param)
{
	struct mpu *mpu = (struct mpu *)param;

	if (gpio_get_level(mpu->drdy_pin)) { // Rising edge
		mpu->drdy = false;
	} else { // Falling edge
		mpu->drdy = true;
		xSemaphoreGiveFromISR(mpu->drdy_semph, NULL);
	}
}

esp_err_t mpu_read_sensors(struct raw_sensors *raw_sensors, struct mpu *mpu)
{
	esp_err_t err;

	if (!mpu->drdy) {
		if (!xSemaphoreTake(mpu->drdy_semph, 1000 / portTICK_RATE_MS))
			mpu->drdy = true; // Assume DTRDY low after timeout
	}

	err = rx((char *)raw_sensors, MPU_SENSORS,
		sizeof(struct raw_sensors), mpu);
	if (err != ESP_OK)
		return err;

	return err;
}

esp_err_t mpu_config(const struct mpu *mpu)
{
	esp_err_t err;

	// Reset
	err = write_reg(MPU_PWR_MGMT_1, MPU_B_DEVICE_RESET, mpu);
	if (err != ESP_OK)
		return err;

	vTaskDelay(10 / portTICK_RATE_MS);

	// Clock select
	err = write_reg(MPU_PWR_MGMT_1, MPU_B_CLK_SEL_GYRZ, mpu);
	if (err != ESP_OK)
		return err;

	vTaskDelay(10 / portTICK_RATE_MS);

	return err;
}

static esp_err_t rx(char *data, char reg_addr, size_t size,
	const struct mpu *mpu)
{
	i2c_cmd_handle_t cmd;
	esp_err_t err;

	cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd,
		(mpu->device_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK);
	i2c_master_write_byte(cmd, reg_addr, ACK_CHECK);

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd,
		(mpu->device_addr << 1) | I2C_MASTER_READ, ACK_CHECK);

	if (size > 1)
		i2c_master_read(cmd, (uint8_t *)data, size - 1, ACK);
	i2c_master_read_byte(cmd, (uint8_t *)(data + size - 1), NACK);

	i2c_master_stop(cmd);

	err = i2c_master_cmd_begin(mpu->i2c_num, cmd, 1000/portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	return err;
}

static esp_err_t tx(char reg_addr, char *data, size_t size,
	const struct mpu *mpu)
{
	i2c_cmd_handle_t cmd;
	esp_err_t err;

	cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd,
		(mpu->device_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK);
	i2c_master_write_byte(cmd, reg_addr, ACK_CHECK);

	i2c_master_write(cmd, (uint8_t *)data, size, ACK_CHECK);

	i2c_master_stop(cmd);

	err = i2c_master_cmd_begin(mpu->i2c_num, cmd, 1000/portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	return err;
}

esp_err_t write_reg(char addr, char val, const struct mpu *mpu)
{
	return tx(addr, &val, 1, mpu);
}

esp_err_t read_reg(char *val, char addr, const struct mpu *mpu)
{
	return rx(val, addr, 1, mpu);
}
