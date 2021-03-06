/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../Drivers/lsm9ds1/lsm9ds1_reg.h"
//#include "i2c.h"//TODO
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
	void *hbus;
	uint8_t i2c_address;
	GPIO_TypeDef *cs_port;
	uint16_t cs_pin;
} sensbus_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define DELAY 10000
#define SENSOR_BUS I2C1
#define BOOT_TIME 20 //ms
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */

static sensbus_t mag_bus = { I2C1,
LSM9DS1_MAG_I2C_ADD_H, 0, 0 };
static sensbus_t imu_bus = { I2C1,
LSM9DS1_IMU_I2C_ADD_H, 0, 0 };

static uint8_t * START_READING = "read\n";
uint8_t OUT_X_GB[1] = { 0 };
uint8_t OUT_X_GL[1] = { 0 };
uint8_t OUT_Y_GB[1] = { 0 };
uint8_t OUT_Y_GL[1] = { 0 };
uint8_t OUT_Z_GB[1] = { 0 };
uint8_t OUT_Z_GL[1] = { 0 };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN PFP */
static int32_t platform_write_imu(void *handle, uint8_t reg,
		const uint8_t *bufp, uint16_t len);
static int32_t platform_read_imu(void *handle, uint8_t reg, uint8_t *bufp,
		uint16_t len);
static int32_t platform_write_mag(void *handle, uint8_t reg,
		const uint8_t *bufp, uint16_t len);
static int32_t platform_read_mag(void *handle, uint8_t reg, uint8_t *bufp,
		uint16_t len);
static void putty_print(uint8_t *tx_buffer, uint16_t len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile int server_message_received = 0; //TODO - flag is set in UART receive method when interrupt is called
static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_magnetic_field[3];
static float acceleration_mg[3];
static float angular_rate_mdps[3];
static float magnetic_field_mgauss[3];
static lsm9ds1_id_t whoamI;
static lsm9ds1_status_t reg;
static uint8_t rst;
static uint8_t tx_buffer[1000];
stmdev_ctx_t dev_ctx_imu;
stmdev_ctx_t dev_ctx_mag;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART6_UART_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */

	init_lsm9ds1();
//	init_lsm9ds1_2();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		doCycle();
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief USART6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART6_UART_Init(void) {

	/* USER CODE BEGIN USART6_Init 0 */

	/* USER CODE END USART6_Init 0 */

	/* USER CODE BEGIN USART6_Init 1 */

	/* USER CODE END USART6_Init 1 */
	huart6.Instance = USART6;
	huart6.Init.BaudRate = 115200;
	huart6.Init.WordLength = UART_WORDLENGTH_8B;
	huart6.Init.StopBits = UART_STOPBITS_1;
	huart6.Init.Parity = UART_PARITY_NONE;
	huart6.Init.Mode = UART_MODE_TX_RX;
	huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart6.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart6) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART6_Init 2 */

	/* USER CODE END USART6_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : PD13 PD14 PD15 */
	GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void doCycle() {
	if (server_message_received) {
		//TODO processing logic
	}
	toggleLed(GPIO_PIN_SET);
	putty_print(START_READING, sizeof(START_READING));
	for (int i = 0; i < 10; i++) {
		readImu();
	}

	toggleLed(GPIO_PIN_RESET);
	HAL_Delay(DELAY);
}

void toggleLed(GPIO_PinState status) {
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
}

void init_lsm9ds1_2(void) {
	uint8_t iNemoConfig[] = { 0xE3 };
	HAL_I2C_Mem_Write(&hi2c1, 0xD5, 0x10, 1, iNemoConfig, 1, 10);
}

void init_lsm9ds1(void) {
	uint8_t iNemoConfig[] = { 0xE3 };
	HAL_I2C_Mem_Write(&hi2c1, 0xD5, 0x10, 1, iNemoConfig, 1, 10);

	/* Initialize inertial sensors (IMU) driver interface */
	dev_ctx_imu.write_reg = platform_write_imu;
	dev_ctx_imu.read_reg = platform_read_imu;
	dev_ctx_imu.handle = (void*) &imu_bus;

	/* Initialize magnetic sensors driver interface */
	dev_ctx_mag.write_reg = platform_write_mag;
	dev_ctx_mag.read_reg = platform_read_mag;
	dev_ctx_mag.handle = (void*) &mag_bus;
	/* Wait sensor boot time */
	HAL_Delay(BOOT_TIME);
	/* Check device ID */
	lsm9ds1_dev_id_get(&dev_ctx_mag, &dev_ctx_imu, &whoamI);

	/* Restore default configuration */
	lsm9ds1_dev_reset_set(&dev_ctx_mag, &dev_ctx_imu, PROPERTY_ENABLE);

	do {
		lsm9ds1_dev_reset_get(&dev_ctx_mag, &dev_ctx_imu, &rst);
	} while (rst);

	/* Enable Block Data Update */
	lsm9ds1_block_data_update_set(&dev_ctx_mag, &dev_ctx_imu, PROPERTY_ENABLE);
	/* Set full scale */
	lsm9ds1_xl_full_scale_set(&dev_ctx_imu, LSM9DS1_4g);
	lsm9ds1_gy_full_scale_set(&dev_ctx_imu, LSM9DS1_2000dps);
	/* Configure filtering chain - See datasheet for filtering chain details */
	/* Accelerometer filtering chain */
	lsm9ds1_xl_filter_aalias_bandwidth_set(&dev_ctx_imu, LSM9DS1_AUTO);
	lsm9ds1_xl_filter_lp_bandwidth_set(&dev_ctx_imu, LSM9DS1_LP_ODR_DIV_50);
	lsm9ds1_xl_filter_out_path_set(&dev_ctx_imu, LSM9DS1_LP_OUT);
	/* Gyroscope filtering chain */
	lsm9ds1_gy_filter_lp_bandwidth_set(&dev_ctx_imu, LSM9DS1_LP_ULTRA_LIGHT);
	lsm9ds1_gy_filter_hp_bandwidth_set(&dev_ctx_imu, LSM9DS1_HP_MEDIUM);
	lsm9ds1_gy_filter_out_path_set(&dev_ctx_imu, LSM9DS1_LPF1_HPF_LPF2_OUT);
	/* Set Output Data Rate / Power mode */
	lsm9ds1_imu_data_rate_set(&dev_ctx_imu, LSM9DS1_IMU_59Hz5);
	/* Read samples in polling mode (no int) */
}

void readImu_2() {

	HAL_I2C_Mem_Read(&hi2c1, 0xD5, 0x28, 1, OUT_X_GB, 1, 10);
	HAL_I2C_Mem_Read(&hi2c1, 0xD5, 0x29, 1, OUT_X_GL, 1, 10);

	HAL_I2C_Mem_Read(&hi2c1, 0xD5, 0x2A, 1, OUT_Y_GB, 1, 10);
	HAL_I2C_Mem_Read(&hi2c1, 0xD5, 0x2B, 1, OUT_Y_GL, 1, 10);

	HAL_I2C_Mem_Read(&hi2c1, 0xD5, 0x2C, 1, OUT_Z_GB, 1, 10);
	HAL_I2C_Mem_Read(&hi2c1, 0xD5, 0x2D, 1, OUT_Z_GL, 1, 10);
}

void printImu() {
	sprintf((char*) tx_buffer,
//				"IMU - [X_GB]:%4.2f\t[X_GL]%4.2f\t[Y_GB]%4.2f\t[Y_GL]:%4.2f\t[Z_GB]%4.2f\t[Z_GL]%4.2f\r\n",
			"IMU - [X_GB]:%d\t[X_GL]%d\t[Y_GB]%d\t[Y_GL]:%d\t[Z_GB]%d\t[Z_GL]%d\r\n",
			OUT_X_GB[0], OUT_X_GL[0], OUT_Y_GB[0], OUT_Y_GL[0], OUT_Z_GB[0],
			OUT_Z_GL[0]);
	putty_print(tx_buffer, strlen((char const*) tx_buffer));
}

void readImu() {

	/* Read device status register */
	lsm9ds1_dev_status_get(&dev_ctx_mag, &dev_ctx_imu, &reg);

	/* Read imu data */
	memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
	memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
	lsm9ds1_acceleration_raw_get(&dev_ctx_imu, data_raw_acceleration);
	lsm9ds1_angular_rate_raw_get(&dev_ctx_imu, data_raw_angular_rate);
	acceleration_mg[0] = lsm9ds1_from_fs4g_to_mg(data_raw_acceleration[0]);
	acceleration_mg[1] = lsm9ds1_from_fs4g_to_mg(data_raw_acceleration[1]);
	acceleration_mg[2] = lsm9ds1_from_fs4g_to_mg(data_raw_acceleration[2]);
	angular_rate_mdps[0] = lsm9ds1_from_fs2000dps_to_mdps(
			data_raw_angular_rate[0]);
	angular_rate_mdps[1] = lsm9ds1_from_fs2000dps_to_mdps(
			data_raw_angular_rate[1]);
	angular_rate_mdps[2] = lsm9ds1_from_fs2000dps_to_mdps(
			data_raw_angular_rate[2]);
	sprintf((char*) tx_buffer,
			"IMU - [mg]:%4.2f\t%4.2f\t%4.2f\t[mdps]:%4.2f\t%4.2f\t%4.2f\r\n",
			acceleration_mg[0], acceleration_mg[1], acceleration_mg[2],
			angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);
	putty_print(tx_buffer, strlen((char const*) tx_buffer));
}

/*
 * @brief  Write generic imu register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write_imu(void *handle, uint8_t reg,
		const uint8_t *bufp, uint16_t len) {
	sensbus_t *sensbus = (sensbus_t*) handle;
	HAL_I2C_Mem_Write(sensbus->hbus, sensbus->i2c_address, reg,
	I2C_MEMADD_SIZE_8BIT, (uint8_t*) bufp, len, 1000);
	return 0;
}

/*
 * @brief  Read generic imu register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read_imu(void *handle, uint8_t reg, uint8_t *bufp,
		uint16_t len) {
	sensbus_t *sensbus = (sensbus_t*) handle;

	HAL_I2C_Mem_Read(sensbus->hbus, sensbus->i2c_address, reg,
	I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
	return 0;
}
/*
 * @brief  Write generic magnetometer register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write_mag(void *handle, uint8_t reg,
		const uint8_t *bufp, uint16_t len) {
	sensbus_t *sensbus = (sensbus_t*) handle;
	/* Write multiple command */
	reg |= 0x80;
	HAL_I2C_Mem_Write(sensbus->hbus, sensbus->i2c_address, reg,
	I2C_MEMADD_SIZE_8BIT, (uint8_t*) bufp, len, 1000);

	return 0;
}

/*
 * @brief  Read generic magnetometer register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read_mag(void *handle, uint8_t reg, uint8_t *bufp,
		uint16_t len) {
	sensbus_t *sensbus = (sensbus_t*) handle;

	/* Read multiple command */
	reg |= 0x80;
	HAL_I2C_Mem_Read(sensbus->hbus, sensbus->i2c_address, reg,
	I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
	return 0;
}

/*
 * @brief  Send buffer to console
 *
 * @param  tx_buffer     buffer to transmit
 * @param  len           number of byte to send
 *
 */
static void putty_print(uint8_t *tx_buffer, uint16_t len) {
	HAL_UART_Transmit(&huart6, tx_buffer, len, 1000);
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	uint8_t *data = { "error!" };
	HAL_UART_Transmit(&huart6, data, sizeof(data), 2000);
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
