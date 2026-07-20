#include "myiic.h"
#include "hw_delay.h"

#define IIC_HW_INST              I2C0
#define IIC_HW_BUSCLK_HZ         40000000U
#define IIC_HW_SPEED_HZ          400000U
#define IIC_HW_TPR               ((IIC_HW_BUSCLK_HZ / (IIC_HW_SPEED_HZ * 10U)) - 1U)
#define IIC_HW_TIMEOUT           200000U
#define IIC_HW_MAX_TX_LEN        4095U

static uint8_t g_iic_hw_inited = 0;

static void IIC_HW_PinInit(void)
{
	DL_GPIO_initPeripheralInputFunctionFeatures(OLED_SDA_IOMUX,
		IOMUX_PINCM1_PF_I2C0_SDA, DL_GPIO_INVERSION_DISABLE,
		DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_ENABLE,
		DL_GPIO_WAKEUP_DISABLE);
	DL_GPIO_enableHiZ(OLED_SDA_IOMUX);

	DL_GPIO_initPeripheralInputFunctionFeatures(OLED_SCL_IOMUX,
		IOMUX_PINCM2_PF_I2C0_SCL, DL_GPIO_INVERSION_DISABLE,
		DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_ENABLE,
		DL_GPIO_WAKEUP_DISABLE);
	DL_GPIO_enableHiZ(OLED_SCL_IOMUX);
}

static void IIC_HW_Init(void)
{
	static const DL_I2C_ClockConfig clockConfig = {
		.clockSel    = DL_I2C_CLOCK_BUSCLK,
		.divideRatio = DL_I2C_CLOCK_DIVIDE_1
	};

	IIC_HW_PinInit();
	DL_I2C_reset(IIC_HW_INST);
	DL_I2C_enablePower(IIC_HW_INST);
	delay_cycles(POWER_STARTUP_DELAY);

	DL_I2C_disableController(IIC_HW_INST);
	DL_I2C_setClockConfig(IIC_HW_INST, &clockConfig);
	DL_I2C_enableAnalogGlitchFilter(IIC_HW_INST);
	DL_I2C_setDigitalGlitchFilterPulseWidth(IIC_HW_INST,
		DL_I2C_DIGITAL_GLITCH_FILTER_WIDTH_CLOCKS_8);
	DL_I2C_resetControllerTransfer(IIC_HW_INST);
	DL_I2C_setTimerPeriod(IIC_HW_INST, IIC_HW_TPR);
	DL_I2C_setControllerTXFIFOThreshold(IIC_HW_INST,
		DL_I2C_TX_FIFO_LEVEL_EMPTY);
	DL_I2C_setControllerRXFIFOThreshold(IIC_HW_INST,
		DL_I2C_RX_FIFO_LEVEL_BYTES_1);
	DL_I2C_enableControllerClockStretching(IIC_HW_INST);
	DL_I2C_enableController(IIC_HW_INST);

	g_iic_hw_inited = 1U;
}

static uint8_t IIC_HW_WaitIdle(void)
{
	uint32_t timeout = IIC_HW_TIMEOUT;
	uint32_t status;

	do
	{
		status = DL_I2C_getControllerStatus(IIC_HW_INST);
		if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
		{
			return 1U;
		}
		if (timeout == 0U)
		{
			return 1U;
		}
		timeout--;
	} while (((status & DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) ||
		((status & DL_I2C_CONTROLLER_STATUS_BUSY_BUS) != 0U));

	return 0U;
}

static uint8_t IIC_HW_GetWriteByte(uint8_t reg_addr, const uint8_t *data,
	uint16_t index)
{
	if (index == 0U)
	{
		return reg_addr;
	}

	return data[index - 1U];
}

uint8_t IIC_HW_WriteReg(uint8_t dev_addr, uint8_t reg_addr,
	const uint8_t *data, uint16_t len)
{
	uint16_t tx_len = len + 1U;
	uint16_t tx_cnt = 0U;
	uint32_t timeout;
	uint32_t status;

	if ((tx_len > IIC_HW_MAX_TX_LEN) || (data == NULL && len > 0U))
	{
		return 1U;
	}
	if (g_iic_hw_inited == 0U)
	{
		IIC_HW_Init();
	}
	if (IIC_HW_WaitIdle() != 0U)
	{
		return 1U;
	}

	DL_I2C_flushControllerTXFIFO(IIC_HW_INST);
	while ((tx_cnt < tx_len) &&
		(DL_I2C_isControllerTXFIFOFull(IIC_HW_INST) == false))
	{
		DL_I2C_transmitControllerData(IIC_HW_INST,
			IIC_HW_GetWriteByte(reg_addr, data, tx_cnt));
		tx_cnt++;
	}

	DL_I2C_startControllerTransfer(IIC_HW_INST, dev_addr,
		DL_I2C_CONTROLLER_DIRECTION_TX, tx_len);

	timeout = IIC_HW_TIMEOUT;
	do
	{
		status = DL_I2C_getControllerStatus(IIC_HW_INST);
		if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
		{
			return 1U;
		}
		if (timeout == 0U)
		{
			return 1U;
		}
		timeout--;
	} while ((status & DL_I2C_CONTROLLER_STATUS_BUSY_BUS) == 0U);

	timeout = IIC_HW_TIMEOUT;
	while ((DL_I2C_getControllerStatus(IIC_HW_INST) &
		DL_I2C_CONTROLLER_STATUS_BUSY_BUS) != 0U)
	{
		uint32_t st = DL_I2C_getControllerStatus(IIC_HW_INST);
		if ((st & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
		{
			return 1U;
		}

		while ((tx_cnt < tx_len) &&
			(DL_I2C_isControllerTXFIFOFull(IIC_HW_INST) == false))
		{
			DL_I2C_transmitControllerData(IIC_HW_INST,
				IIC_HW_GetWriteByte(reg_addr, data, tx_cnt));
			tx_cnt++;
		}

		if (timeout == 0U)
		{
			return 1U;
		}
		timeout--;
	}

	if ((DL_I2C_getControllerStatus(IIC_HW_INST) &
		DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
	{
		return 1U;
	}

	return 0U;
}

uint8_t IIC_HW_ReadReg(uint8_t dev_addr, uint8_t reg_addr,
	uint8_t *data, uint16_t len)
{
	uint16_t rx_cnt = 0U;
	uint32_t timeout;
	uint32_t status;

	if ((data == NULL) || (len == 0U))
	{
		return 1U;
	}
	if (IIC_HW_WriteReg(dev_addr, reg_addr, NULL, 0U) != 0U)
	{
		return 1U;
	}
	if (IIC_HW_WaitIdle() != 0U)
	{
		return 1U;
	}

	DL_I2C_flushControllerRXFIFO(IIC_HW_INST);
	DL_I2C_startControllerTransfer(IIC_HW_INST, dev_addr,
		DL_I2C_CONTROLLER_DIRECTION_RX, len);

	timeout = IIC_HW_TIMEOUT;
	do
	{
		status = DL_I2C_getControllerStatus(IIC_HW_INST);
		if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
		{
			return 1U;
		}
		if (timeout == 0U)
		{
			return 1U;
		}
		timeout--;
	} while ((status & DL_I2C_CONTROLLER_STATUS_BUSY_BUS) == 0U);

	timeout = IIC_HW_TIMEOUT;
	while (rx_cnt < len)
	{
		status = DL_I2C_getControllerStatus(IIC_HW_INST);
		if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
		{
			return 1U;
		}
		if (DL_I2C_isControllerRXFIFOEmpty(IIC_HW_INST) == false)
		{
			data[rx_cnt] = DL_I2C_receiveControllerData(IIC_HW_INST);
			rx_cnt++;
			timeout = IIC_HW_TIMEOUT;
			continue;
		}
		if (timeout == 0U)
		{
			return 1U;
		}
		timeout--;
	}

	return 0U;
}

// 产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT(); // sda线输出
	IIC_SDA(1);
	IIC_SCL(1);
	delay_us(4);
	IIC_SDA(0); // START:when CLK is high,DATA change form high to low
	delay_us(4);
	IIC_SCL(0); // 钳住I2C总线，准备发送或接收数据
}
// 产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT(); // sda线输出
	IIC_SCL(0);
	IIC_SDA(0); // STOP:when CLK is high DATA change form low to high
	delay_us(4);
	IIC_SCL(1);
	IIC_SDA(1); // 发送I2C总线结束信号
	delay_us(4);
}
// 等待应答信号到来
// 返回值：1，接收应答失败
//         0，接收应答成功
unsigned char IIC_Wait_Ack(void)
{
	unsigned char ucErrTime = 0;
	SDA_IN(); // SDA设置为输入
	IIC_SDA(1);
	delay_us(1);
	IIC_SCL(1);
	delay_us(1);
	while (SDA_GET())
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL(0); // 时钟输出0
	return 0;
}
// 产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(0);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}
// 不产生ACK应答
void IIC_NAck(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(1);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}
// IIC发送一个字节
// 返回从机有无应答
// 1，有应答
// 0，无应答
void IIC_Send_Byte(unsigned char txd)
{
	unsigned char t;
	SDA_OUT();
	IIC_SCL(0); // 拉低时钟开始数据传输
	for (t = 0; t < 8; t++)
	{
		IIC_SDA( (txd & 0x80) >> 7 );
		txd <<= 1;
		delay_us(2); // 对TEA5767这三个延时都是必须的
		IIC_SCL(1);
		delay_us(2);
		IIC_SCL(0);
		delay_us(2);
	}
}
// 读1个字节，ack=1时，发送ACK，ack=0，发送nACK
unsigned char IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	SDA_IN(); // SDA设置为输入
	for (i = 0; i < 8; i++)
	{
		IIC_SCL(0);
		delay_us(2);
		IIC_SCL(1);
		receive <<= 1;
		if (SDA_GET())
			receive++;
		delay_us(1);
	}
	if (!ack)
		IIC_NAck(); // 发送nACK
	else
		IIC_Ack(); // 发送ACK
	return receive;
}
