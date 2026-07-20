/**
  ******************************************************************************
  * @file    OLED_driver.c
  * @brief   OLED 底层驱动文件
  * @note    移植到其他芯片时，优先修改文件前面的“用户配置区”和
  *          “底层移植区”。后面的 OLED 命令、数据写入、刷新逻辑一般不用改。
  *
  *          移植步骤：
  *          1. 在 OLED_driver.h 中修改平台头文件和 I2C 方式。
  *          2. 在“用户配置区”修改 OLED 地址、I2C 速度、超时参数。
  *          3. 在“底层移植区”适配 SCL/SDA 引脚操作函数。
  *          4. 如果使用新芯片的硬件 I2C，替换“硬件 I2C 驱动区”。
  ******************************************************************************
  */
#include "OLED_driver.h"


/* OLED 屏幕宽度，单位：像素 */
#define OLED_WIDTH                 128U

/* OLED 屏幕高度，单位：像素 */
#define OLED_HEIGHT                64U

/* OLED 每页高度，SSD1306 一页为 8 行 */
#define OLED_PAGE_HEIGHT           8U

/* OLED 页数 */
#define OLED_PAGE_NUM              (OLED_HEIGHT / OLED_PAGE_HEIGHT)

/* OLED 最小亮度值 */
#define OLED_BRIGHTNESS_MIN        0

/* OLED 最大亮度值 */
#define OLED_BRIGHTNESS_MAX        255

uint8_t OLED_DisplayBuf[OLED_PAGE_NUM][OLED_WIDTH];

bool OLED_ColorMode = true;
static uint8_t OLED_LastDisplayBuf[OLED_PAGE_NUM][OLED_WIDTH];
static bool OLED_LastDisplayValid = false;

/*============================ 用户配置区 ============================*/

/* OLED 8 位写地址，7 位地址 0x3C 左移 1 位 */
#define OLED_I2C_ADDR_WRITE        0x78U

/* I2C 写失败后的重试次数 */
#define OLED_I2C_RETRY_COUNT       2U

/* 等待 SCL/SDA 释放的超时计数 */
#define OLED_I2C_LINE_TIMEOUT      1000U

/* 总线恢复时补发的 SCL 时钟数 */
#define OLED_I2C_RECOVERY_CLOCKS   9U

/* I2C 一个字节包含的位数 */
#define OLED_I2C_BYTE_BITS         8U

/* 模拟 I2C 延时，约 1us */
#define OLED_I2C_DELAY_CYCLES      (CPUCLK_FREQ / 1000000U)

/* 当前 MSPM0G3519 开发板的硬件 I2C 参数 */
#if (OLED_DRIVER_BUS_BACKEND == OLED_BUS_BACKEND_MSPM0_HW_I2C)
/* 当前使用的 MSPM0 硬件 I2C 外设 */
#define OLED_HW_I2C_INST           I2C0

/* OLED 7 位从机地址 */
#define OLED_HW_I2C_ADDR           0x3CU

/* I2C 外设输入时钟频率 */
#define OLED_HW_I2C_BUSCLK_HZ      40000000U

/* I2C 通信速率，当前稳定值 400kHz */
#define OLED_HW_I2C_SPEED_HZ       400000U

/* MSPM0 I2C 分频值 */
#define OLED_HW_I2C_TPR            ((OLED_HW_I2C_BUSCLK_HZ / (OLED_HW_I2C_SPEED_HZ * 10U)) - 1U)

/* 硬件 I2C 等待超时计数 */
#define OLED_HW_I2C_TIMEOUT        200000U

/* MSPM0 I2C 控制器单次发送长度最大值，包含 1 字节控制字 */
#define OLED_HW_I2C_MAX_TX_LEN     4095U

/* MSPM0 I2C 控制器单次发送数据最大值，不包含控制字 */
#define OLED_HW_I2C_MAX_DATA_LEN   (OLED_HW_I2C_MAX_TX_LEN - 1U)
#endif

/* 脏区刷新时，连续空白列超过该值则拆分传输 */
#define OLED_DIRTY_SPLIT_GAP       16U

static bool OLED_I2C_NeedInit = false;

/*============================ 底层移植区 ============================*/

/*
 * 说明：
 * 1. OLED 上层只调用 OLED_BusInit、OLED_BusWrite、OLED_BusRecover。
 * 2. 移植新芯片时，优先改本区的 GPIO 操作函数。
 * 3. I2C 引脚需要开漏效果：
 *    Release 表示输入/高阻释放总线，Low 表示主动输出低电平。
 */
/**
  * 函    数：OLED_PortDelay
  * 功    能：底层 I2C 时序延时
  * 参    数：无
  * 返 回 值：无
  * 说    明：移植新芯片时，根据主频调整延时实现
  */
static void OLED_PortDelay(void)
{
	delay_cycles(OLED_I2C_DELAY_CYCLES);
}

/**
  * 函    数：OLED_PortSCLRelease
  * 功    能：释放 SCL 总线
  * 参    数：无
  * 返 回 值：无
  * 说    明：I2C 开漏高电平通过释放引脚实现
  */
static void OLED_PortSCLRelease(void)
{
	DL_GPIO_disableOutput(OLED_PORT, OLED_SCL_PIN);
}

/**
  * 函    数：OLED_PortSCLLow
  * 功    能：拉低 SCL 总线
  * 参    数：无
  * 返 回 值：无
  * 说    明：I2C 低电平由主机主动输出低电平
  */
static void OLED_PortSCLLow(void)
{
	DL_GPIO_clearPins(OLED_PORT, OLED_SCL_PIN);
	DL_GPIO_enableOutput(OLED_PORT, OLED_SCL_PIN);
}

/**
  * 函    数：OLED_PortSDARelease
  * 功    能：释放 SDA 总线
  * 参    数：无
  * 返 回 值：无
  * 说    明：I2C 开漏高电平通过释放引脚实现
  */
static void OLED_PortSDARelease(void)
{
	DL_GPIO_disableOutput(OLED_PORT, OLED_SDA_PIN);
}

/**
  * 函    数：OLED_PortSDALow
  * 功    能：拉低 SDA 总线
  * 参    数：无
  * 返 回 值：无
  * 说    明：I2C 低电平由主机主动输出低电平
  */
static void OLED_PortSDALow(void)
{
	DL_GPIO_clearPins(OLED_PORT, OLED_SDA_PIN);
	DL_GPIO_enableOutput(OLED_PORT, OLED_SDA_PIN);
}

/**
  * 函    数：OLED_PortReadSCL
  * 功    能：读取 SCL 当前电平
  * 参    数：无
  * 返 回 值：1：高电平；0：低电平
  * 说    明：用于检测时钟拉伸和总线恢复状态
  */
static uint8_t OLED_PortReadSCL(void)
{
	return (DL_GPIO_readPins(OLED_PORT, OLED_SCL_PIN) != 0U) ? 1U : 0U;
}

/**
  * 函    数：OLED_PortReadSDA
  * 功    能：读取 SDA 当前电平
  * 参    数：无
  * 返 回 值：1：高电平；0：低电平
  * 说    明：用于 ACK 检测和总线恢复状态判断
  */
static uint8_t OLED_PortReadSDA(void)
{
	return (DL_GPIO_readPins(OLED_PORT, OLED_SDA_PIN) != 0U) ? 1U : 0U;
}


/* 模拟 I2C 函数体在后面，硬件 I2C 恢复和软件总线会调用这些函数 */
static bool OLED_SoftI2C_RecoverBus(void);
static bool OLED_SoftI2C_Start(void);
static void OLED_SoftI2C_Stop(void);
static bool OLED_SoftI2C_SendByte(uint8_t Byte);
/*============================ 硬件 I2C 驱动区 ============================*/

/*
 * 当前实现：MSPM0G3519，PA0/PA1，I2C0。
 *
 * 移植到其他芯片的硬件 I2C 时，只需要保证下方三个接口语义不变：
 *   OLED_BusInit    ：初始化 I2C 外设和引脚
 *   OLED_BusWrite   ：发送一包 OLED 数据，成功返回 true，失败返回 false
 *   OLED_BusRecover ：恢复异常 I2C 总线
 */
#if (OLED_DRIVER_BUS_BACKEND == OLED_BUS_BACKEND_MSPM0_HW_I2C)
/**
  * 函    数：OLED_MSPM0_I2C_PinInit
  * 功    能：初始化 MSPM0 硬件 I2C 引脚复用
  * 参    数：无
  * 返 回 值：无
  * 说    明：当前工程使用 PA0/PA1 作为 I2C0 的 SDA/SCL
  */
static void OLED_MSPM0_I2C_PinInit(void)
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


/**
  * 函    数：OLED_MSPM0_I2C_ModuleInit
  * 功    能：初始化 MSPM0 I2C 控制器
  * 参    数：无
  * 返 回 值：无
  * 说    明：包含时钟、滤波、分频、FIFO 阈值和时钟拉伸配置
  */
static void OLED_MSPM0_I2C_ModuleInit(void)
{
	static const DL_I2C_ClockConfig clockConfig = {
		.clockSel    = DL_I2C_CLOCK_BUSCLK,
		.divideRatio = DL_I2C_CLOCK_DIVIDE_1
	};

	DL_I2C_reset(OLED_HW_I2C_INST);
	DL_I2C_enablePower(OLED_HW_I2C_INST);
	delay_cycles(POWER_STARTUP_DELAY);

	DL_I2C_disableController(OLED_HW_I2C_INST);
	DL_I2C_setClockConfig(OLED_HW_I2C_INST, &clockConfig);
	DL_I2C_enableAnalogGlitchFilter(OLED_HW_I2C_INST);
	DL_I2C_setDigitalGlitchFilterPulseWidth(OLED_HW_I2C_INST,
		DL_I2C_DIGITAL_GLITCH_FILTER_WIDTH_CLOCKS_8);
	DL_I2C_resetControllerTransfer(OLED_HW_I2C_INST);
	DL_I2C_setTimerPeriod(OLED_HW_I2C_INST, OLED_HW_I2C_TPR);
	DL_I2C_setControllerTXFIFOThreshold(OLED_HW_I2C_INST,
		DL_I2C_TX_FIFO_LEVEL_EMPTY);
	DL_I2C_setControllerRXFIFOThreshold(OLED_HW_I2C_INST,
		DL_I2C_RX_FIFO_LEVEL_BYTES_1);
	DL_I2C_enableControllerClockStretching(OLED_HW_I2C_INST);
	DL_I2C_enableController(OLED_HW_I2C_INST);
}


/**
  * 函    数：OLED_MSPM0_I2C_Init
  * 功    能：初始化 MSPM0 硬件 I2C 总线
  * 参    数：无
  * 返 回 值：无
  * 说    明：先配置引脚复用，再配置 I2C 外设
  */
static void OLED_MSPM0_I2C_Init(void)
{
	OLED_MSPM0_I2C_PinInit();
	OLED_MSPM0_I2C_ModuleInit();
}


/**
  * 函    数：OLED_MSPM0_I2C_WaitIdle
  * 功    能：等待 MSPM0 I2C 控制器空闲
  * 参    数：无
  * 返 回 值：true：空闲；false：错误或超时
  * 说    明：发送新数据前必须确认总线和控制器均为空闲
  */
static bool OLED_MSPM0_I2C_WaitIdle(void)
{
	uint32_t timeout = OLED_HW_I2C_TIMEOUT;
	uint32_t status;

	do
	{
		status = DL_I2C_getControllerStatus(OLED_HW_I2C_INST);
		if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
		{
			return false;
		}
		if (timeout == 0U)
		{
			return false;
		}
		timeout--;
	} while (((status & DL_I2C_CONTROLLER_STATUS_IDLE) == 0U) ||
		((status & DL_I2C_CONTROLLER_STATUS_BUSY_BUS) != 0U));

	return true;
}


/**
  * 函    数：OLED_MSPM0_I2C_RecoverBus
  * 功    能：恢复异常的 MSPM0 I2C 总线
  * 参    数：无
  * 返 回 值：true：恢复成功；false：恢复失败
  * 说    明：先关闭硬件 I2C，再临时切 GPIO 补时钟，最后重新初始化硬件 I2C
  */
static bool OLED_MSPM0_I2C_RecoverBus(void)
{
	bool recovered;

	DL_I2C_disableController(OLED_HW_I2C_INST);

	DL_GPIO_initDigitalInputFeatures(OLED_SDA_IOMUX,
		DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		DL_GPIO_HYSTERESIS_ENABLE, DL_GPIO_WAKEUP_DISABLE);
	DL_GPIO_initDigitalInputFeatures(OLED_SCL_IOMUX,
		DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		DL_GPIO_HYSTERESIS_ENABLE, DL_GPIO_WAKEUP_DISABLE);
	DL_GPIO_enableOutput(OLED_PORT, OLED_SCL_PIN);
	DL_GPIO_enableOutput(OLED_PORT, OLED_SDA_PIN);
	DL_GPIO_setPins(OLED_PORT, OLED_SCL_PIN | OLED_SDA_PIN);

	recovered = OLED_SoftI2C_RecoverBus();
	OLED_MSPM0_I2C_Init();

	return recovered;
}


/**
  * 函    数：OLED_MSPM0_I2C_GetStreamByte
  * 功    能：获取硬件 I2C 连续发送流中的指定字节
  * 参    数：Control：OLED 控制字节，0x00 为命令，0x40 为数据
  * 参    数：Data：待发送的数据缓冲区
  * 参    数：Index：发送流中的字节序号
  * 参    数：Invert：是否对数据取反
  * 返 回 值：需要写入 I2C TX FIFO 的字节
  * 说    明：Index 为 0 时返回控制字节，后续返回 Data 中的数据
  */
static uint8_t OLED_MSPM0_I2C_GetStreamByte(uint8_t Control, const uint8_t *Data,
	uint16_t Index, bool Invert)
{
	uint8_t data;

	if (Index == 0U)
	{
		return Control;
	}

	data = Data[Index - 1U];
	return Invert ? (uint8_t)(~data) : data;
}


/**
  * 函    数：OLED_MSPM0_I2C_Write
  * 功    能：通过 MSPM0 硬件 I2C 发送 OLED 数据
  * 参    数：Control：OLED 控制字节，0x00 为命令，0x40 为数据
  * 参    数：Data：待发送的数据缓冲区
  * 参    数：Count：待发送的数据长度，不包含控制字节
  * 参    数：Invert：是否对数据取反
  * 返 回 值：true：发送成功；false：发送失败
  * 说    明：发送失败时会触发总线恢复，并置位 OLED_I2C_NeedInit
  */
static bool OLED_MSPM0_I2C_Write(uint8_t Control, const uint8_t *Data,
	uint16_t Count, bool Invert)
{
	uint16_t len = Count + 1U;
	uint16_t txCount;
	uint32_t timeout;
	uint32_t status;

	if (Count > OLED_HW_I2C_MAX_DATA_LEN)
	{
		Count = OLED_HW_I2C_MAX_DATA_LEN;
		len = OLED_HW_I2C_MAX_TX_LEN;
	}

	if (!OLED_MSPM0_I2C_WaitIdle())
	{
		OLED_MSPM0_I2C_RecoverBus();
	}

	DL_I2C_flushControllerTXFIFO(OLED_HW_I2C_INST);
	txCount = 0U;
	while ((txCount < len) &&
		(DL_I2C_isControllerTXFIFOFull(OLED_HW_I2C_INST) == false))
	{
		DL_I2C_transmitControllerData(OLED_HW_I2C_INST,
			OLED_MSPM0_I2C_GetStreamByte(Control, Data, txCount, Invert));
		txCount++;
	}

	DL_I2C_startControllerTransfer(OLED_HW_I2C_INST, OLED_HW_I2C_ADDR,
		DL_I2C_CONTROLLER_DIRECTION_TX, len);

	timeout = OLED_HW_I2C_TIMEOUT;
	do
	{
		status = DL_I2C_getControllerStatus(OLED_HW_I2C_INST);
		if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
		{
			OLED_MSPM0_I2C_RecoverBus();
			OLED_I2C_NeedInit = true;
			return false;
		}
		if (timeout == 0U)
		{
			OLED_MSPM0_I2C_RecoverBus();
			OLED_I2C_NeedInit = true;
			return false;
		}
		timeout--;
	} while ((status & DL_I2C_CONTROLLER_STATUS_BUSY_BUS) == 0U);

	timeout = OLED_HW_I2C_TIMEOUT;
	while ((DL_I2C_getControllerStatus(OLED_HW_I2C_INST) &
		DL_I2C_CONTROLLER_STATUS_BUSY_BUS) != 0U)
	{
		uint32_t status = DL_I2C_getControllerStatus(OLED_HW_I2C_INST);
		if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
		{
			OLED_MSPM0_I2C_RecoverBus();
			OLED_I2C_NeedInit = true;
			return false;
		}

		while ((txCount < len) &&
			(DL_I2C_isControllerTXFIFOFull(OLED_HW_I2C_INST) == false))
		{
			DL_I2C_transmitControllerData(OLED_HW_I2C_INST,
				OLED_MSPM0_I2C_GetStreamByte(Control, Data, txCount, Invert));
			txCount++;
		}

		if (timeout == 0U)
		{
			OLED_MSPM0_I2C_RecoverBus();
			OLED_I2C_NeedInit = true;
			return false;
		}
		timeout--;
	}

	if ((DL_I2C_getControllerStatus(OLED_HW_I2C_INST) &
		DL_I2C_CONTROLLER_STATUS_ERROR) != 0U)
	{
		OLED_MSPM0_I2C_RecoverBus();
		OLED_I2C_NeedInit = true;
		return false;
	}

	return true;
}


/**
  * 函    数：OLED_BusInit
  * 功    能：初始化 OLED 总线
  * 参    数：无
  * 返 回 值：无
  * 说    明：硬件 I2C 后端下初始化 MSPM0 I2C 外设
  */
static void OLED_BusInit(void)
{
	OLED_MSPM0_I2C_Init();
}


/**
  * 函    数：OLED_BusRecover
  * 功    能：恢复 OLED 总线
  * 参    数：无
  * 返 回 值：true：恢复成功；false：恢复失败
  * 说    明：硬件 I2C 后端下执行 MSPM0 I2C 总线恢复
  */
static bool OLED_BusRecover(void)
{
	return OLED_MSPM0_I2C_RecoverBus();
}


/**
  * 函    数：OLED_BusWrite
  * 功    能：向 OLED 总线写入一包数据
  * 参    数：Control：OLED 控制字节，0x00 为命令，0x40 为数据
  * 参    数：Data：待发送的数据缓冲区
  * 参    数：Count：待发送的数据长度，不包含控制字节
  * 参    数：Invert：是否对数据取反
  * 返 回 值：true：发送成功；false：发送失败
  * 说    明：该接口屏蔽底层使用硬件 I2C 还是模拟 I2C
  */
static bool OLED_BusWrite(uint8_t Control, const uint8_t *Data,
	uint16_t Count, bool Invert)
{
	return OLED_MSPM0_I2C_Write(Control, Data, Count, Invert);
}

#else

/* 使用模拟 I2C 作为 OLED 总线 */
/**
  * 函    数：OLED_BusInit
  * 功    能：初始化 OLED 总线
  * 参    数：无
  * 返 回 值：无
  * 说    明：模拟 I2C 后端下释放 SCL/SDA 并尝试恢复总线
  */
static void OLED_BusInit(void)
{
	OLED_PortSCLRelease();
	OLED_PortSDARelease();
	(void)OLED_SoftI2C_RecoverBus();
}


/**
  * 函    数：OLED_BusRecover
  * 功    能：恢复 OLED 总线
  * 参    数：无
  * 返 回 值：true：恢复成功；false：恢复失败
  * 说    明：模拟 I2C 后端下通过补时钟释放异常从机
  */
static bool OLED_BusRecover(void)
{
	return OLED_SoftI2C_RecoverBus();
}


/**
  * 函    数：OLED_BusWrite
  * 功    能：向 OLED 总线写入一包数据
  * 参    数：Control：OLED 控制字节，0x00 为命令，0x40 为数据
  * 参    数：Data：待发送的数据缓冲区
  * 参    数：Count：待发送的数据长度，不包含控制字节
  * 参    数：Invert：是否对数据取反
  * 返 回 值：true：发送成功；false：发送失败
  * 说    明：模拟 I2C 后端会检查每个字节 ACK，失败后自动重试
  */
static bool OLED_BusWrite(uint8_t Control, const uint8_t *Data,
	uint16_t Count, bool Invert)
{
	uint8_t retry;
	uint16_t i;

	for (retry = 0U; retry <= OLED_I2C_RETRY_COUNT; retry++)
	{
		bool ok = OLED_SoftI2C_Start();
		ok = ok && OLED_SoftI2C_SendByte(OLED_I2C_ADDR_WRITE);
		ok = ok && OLED_SoftI2C_SendByte(Control);

		for (i = 0U; ok && (i < Count); i++)
		{
			uint8_t byte = Data[i];
			if (Invert)
			{
				byte = (uint8_t)(~byte);
			}
			ok = OLED_SoftI2C_SendByte(byte);
		}

		OLED_SoftI2C_Stop();
		if (ok)
		{
			return true;
		}
		(void)OLED_BusRecover();
	}

	OLED_I2C_NeedInit = true;
	return false;
}

#endif




/*============================ 以下为内部实现，移植时一般不用修改 ============================*/

/*============================ 模拟 I2C 驱动区 ============================*/

/**
  * 函    数：OLED_SoftI2C_WaitSCLHigh
  * 功    能：等待 SCL 释放为高电平
  * 参    数：无
  * 返 回 值：true：SCL 已变为高电平；false：等待超时
  * 说    明：支持从机时钟拉伸检测，也用于总线恢复过程
  */
static bool OLED_SoftI2C_WaitSCLHigh(void)
{
	uint16_t timeout = OLED_I2C_LINE_TIMEOUT;

	OLED_PortSCLRelease();
	while (OLED_PortReadSCL() == 0U)
	{
		if (timeout == 0U)
		{
			return false;
		}
		timeout--;
		OLED_PortDelay();
	}

	OLED_PortDelay();
	return true;
}

/**
  * 函    数：OLED_SoftI2C_PulseSCL
  * 功    能：产生一个模拟 I2C SCL 时钟脉冲
  * 参    数：无
  * 返 回 值：true：时钟产生成功；false：SCL 被拉低超时
  * 说    明：发送数据位和总线恢复时使用
  */
static bool OLED_SoftI2C_PulseSCL(void)
{
	OLED_PortSCLRelease();
	OLED_PortDelay();
	if (OLED_PortReadSCL() == 0U)
	{
		if (!OLED_SoftI2C_WaitSCLHigh())
		{
			return false;
		}
	}
	OLED_PortSCLLow();
	OLED_PortDelay();
	return true;
}


/**
  * 函    数：OLED_SoftI2C_RecoverBus
  * 功    能：恢复异常的模拟 I2C 总线
  * 参    数：无
  * 返 回 值：true：恢复成功；false：恢复失败
  * 说    明：补发若干 SCL 时钟后发送停止信号，释放可能卡住的从机
  */
static bool OLED_SoftI2C_RecoverBus(void)
{
	uint8_t i;

	OLED_PortSDARelease();
	OLED_PortSCLRelease();
	OLED_PortDelay();

	/* 从机可能停在一个字节的中间，补 9 个时钟后再发送停止信号释放总线。 */
	for (i = 0U; i < OLED_I2C_RECOVERY_CLOCKS; i++)
	{
		if ((OLED_PortReadSDA() != 0U) && (OLED_PortReadSCL() != 0U))
		{
			break;
		}
		OLED_PortSCLLow();
		OLED_PortDelay();
		if (!OLED_SoftI2C_WaitSCLHigh())
		{
			return false;
		}
	}

	OLED_PortSDALow();
	OLED_PortDelay();
	if (!OLED_SoftI2C_WaitSCLHigh())
	{
		return false;
	}
	OLED_PortSDARelease();
	OLED_PortDelay();

	return (OLED_PortReadSDA() != 0U) && (OLED_PortReadSCL() != 0U);
}


/**
  * 函    数：OLED_SoftI2C_WriteSCL
  * 功    能：控制模拟 I2C 的 SCL 电平
  * 参    数：BitValue，1 释放 SCL，0 拉低 SCL
  * 返 回 值：无
  * 说    明：模拟 I2C 的 SCL 输出控制
  */
static void OLED_SoftI2C_WriteSCL(uint8_t BitValue)
{
	if (BitValue)
	{
		(void)OLED_SoftI2C_WaitSCLHigh();
	}
	else
	{
		OLED_PortSCLLow();
		OLED_PortDelay();
	}
}


/**
  * 函    数：OLED_SoftI2C_WriteSDA
  * 功    能：控制模拟 I2C 的 SDA 电平
  * 参    数：BitValue，1 释放 SDA，0 拉低 SDA
  * 返 回 值：无
  * 说    明：模拟 I2C 的 SDA 输出控制
  */
static void OLED_SoftI2C_WriteSDA(uint8_t BitValue)
{
	if (BitValue)
	{
		OLED_PortSDARelease();
	}
	else
	{
		OLED_PortSDALow();
	}
	OLED_PortDelay();
}


/*通信协议*********************/

/**
  * 函    数：I2C起始
  * 功    能：产生模拟 I2C 起始信号
  * 参    数：无
  * 返 回 值：true：起始信号成功；false：总线异常
  * 说    明：起始前会检查 SDA 是否已被释放
  */
static bool OLED_SoftI2C_Start(void)
{
	OLED_SoftI2C_WriteSDA(1);
	if (!OLED_SoftI2C_WaitSCLHigh())
	{
		return false;
	}
	if (OLED_PortReadSDA() == 0U)
	{
		return false;
	}
	OLED_SoftI2C_WriteSDA(0);
	OLED_SoftI2C_WriteSCL(0);
	return true;
}


/**
  * 函    数：I2C终止
  * 功    能：产生模拟 I2C 停止信号
  * 参    数：无
  * 返 回 值：无
  * 说    明：停止信号用于释放 I2C 总线
  */
static void OLED_SoftI2C_Stop(void)
{
	OLED_SoftI2C_WriteSDA(0);
	(void)OLED_SoftI2C_WaitSCLHigh();
	OLED_SoftI2C_WriteSDA(1);
}



/**
  * 函    数：I2C发送一个字节
  * 功    能：通过模拟 I2C 发送 1 个字节
  * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
  * 返 回 值：true：收到 ACK；false：未收到 ACK 或总线异常
  * 说    明：每发送 8 位后释放 SDA 并读取从机应答
  */
static bool OLED_SoftI2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	bool ack;

	for (i = 0U; i < OLED_I2C_BYTE_BITS; i++)
	{
		OLED_SoftI2C_WriteSDA((Byte & (0x80U >> i)) ? 1U : 0U);
		if (!OLED_SoftI2C_PulseSCL())
		{
			return false;
		}
	}

	OLED_SoftI2C_WriteSDA(1);
	if (!OLED_SoftI2C_WaitSCLHigh())
	{
		return false;
	}
	ack = (OLED_PortReadSDA() == 0U);
	OLED_SoftI2C_WriteSCL(0);
	return ack;
}


/*============================ OLED 通信接口区 ============================*/

/**
  * 函    数：OLED_BusWriteBytes
  * 功    能：通过 OLED 总线发送命令字节序列
  * 参    数：Control：OLED 控制字节，0x00 为命令，0x40 为数据
  * 参    数：Data：待发送的数据缓冲区
  * 参    数：Count：待发送的数据长度，不包含控制字节
  * 返 回 值：true：发送成功；false：发送失败
  * 说    明：命令写入默认不做颜色取反
  */
static bool OLED_BusWriteBytes(uint8_t Control, const uint8_t *Data,
	uint8_t Count)
{
	return OLED_BusWrite(Control, Data, Count, false);
}



/**
  * 函    数：OLED写命令
  * 功    能：向 OLED 写入单个命令
  * 参    数：Command 要写入的命令值，范围：0x00~0xFF
  * 返 回 值：无
  * 说    明：底层通过 OLED_BusWriteBytes 发送
  */
void OLED_WriteCommand(uint8_t Command)
{
	(void)OLED_BusWriteBytes(0x00U, &Command, 1U);
}

/**
  * 函    数：OLED_WriteCommands
  * 功    能：向 OLED 连续写入多个命令
  * 参    数：Commands：命令缓冲区
  * 参    数：Count：命令数量
  * 返 回 值：无
  * 说    明：用于减少初始化和设置光标时的 I2C 起停次数
  */
static void OLED_WriteCommands(const uint8_t *Commands, uint8_t Count)
{
	(void)OLED_BusWriteBytes(0x00U, Commands, Count);
}


/**
  * 函    数：OLED_SetColorMode
  * 功    能：设置 OLED 显示颜色模式
  * 参    数：colormode，true：正常模式；false：反色数据模式
  * 返 回 值：无
  * 说    明：颜色模式改变后会使显示缓存失效，下一次刷新会重新写屏
  */
void OLED_SetColorMode(bool colormode)
{
	if (OLED_ColorMode != colormode)
	{
		OLED_LastDisplayValid = false;
	}
	OLED_ColorMode = colormode;
}
/**
  * 函    数：OLED写数据
  * 功    能：向 OLED 写入显示数据
  * 参    数：Data 要写入数据的起始地址
  * 参    数：Count 要写入数据的数量
  * 返 回 值：无
  * 说    明：当 OLED_ColorMode 为 false 时，发送前会对数据取反
  */
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
	(void)OLED_BusWrite(0x40U, Data, Count, !OLED_ColorMode);
}




/*********************通信协议*/




/**
  * 函    数：OLED引脚初始化
  * 功    能：初始化 OLED 底层通信总线
  * 参    数：无
  * 返 回 值：无
  * 说    明：上电后先延时等待 OLED 稳定，再初始化当前选择的 I2C 总线
  */
void OLED_GPIO_Init(void)
{
	uint32_t i, j;

	/* 首次通信前等待 OLED 供电稳定。 */
	for (i = 0; i < 1000; i++)
	{
		for (j = 0; j < 1000; j++);
	}

	OLED_BusInit();
}


/**
  * 函    数：OLED设置显示光标位置
  * 功    能：设置 OLED 后续写入数据的页地址和列地址
  * 参    数：Page 指定光标所在的页，范围：0~7
  * 参    数：X 指定光标所在的X轴坐标，范围：0~127
  * 返 回 值：无
  * 说    明：OLED默认的Y轴，只能8个Bit为一组写入，即1页等于8个Y轴坐标
  */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
	uint8_t Commands[3];

	/*如果使用此程序驱动1.3寸的OLED显示屏，则需要解除此注释*/
	/*因为1.3寸的OLED驱动芯片（SH1106）有132列*/
	/*屏幕的起始列接在了第2列，而不是第0列*/
	/*所以需要将X加2，才能正常显示*/
//	X += 2;
	
	/*通过指令设置页地址和列地址*/
	Commands[0] = 0xB0U | Page;					//设置页位置
	Commands[1] = 0x10U | ((X & 0xF0U) >> 4);	//设置X位置高4位
	Commands[2] = 0x00U | (X & 0x0FU);			//设置X位置低4位
	OLED_WriteCommands(Commands, sizeof(Commands));
}

/**
  * 函    数：OLED_FlushDirtyRange
  * 功    能：刷新指定页内发生变化的列范围
  * 参    数：Page：页号，范围 0~7
  * 参    数：X：起始列坐标
  * 参    数：Width：刷新宽度
  * 返 回 值：true：刷新成功；false：刷新失败
  * 说    明：通过比较当前显存和上次显存，只发送变化数据，提升刷新速度
  */
static bool OLED_FlushDirtyRange(uint8_t Page, uint8_t X, uint8_t Width)
{
	uint16_t pos;
	uint16_t end;
	uint16_t runStart;
	uint16_t lastDirty;
	uint16_t cleanCount;
	uint16_t runWidth;

	if (Width == 0U)
	{
		return true;
	}

	if (!OLED_LastDisplayValid)
	{
		OLED_SetCursor(Page, X);
		OLED_WriteData(&OLED_DisplayBuf[Page][X], Width);
		if (OLED_I2C_NeedInit)
		{
			OLED_LastDisplayValid = false;
			return false;
		}
		memcpy(&OLED_LastDisplayBuf[Page][X], &OLED_DisplayBuf[Page][X], Width);
		return true;
	}

	pos = X;
	end = (uint16_t)X + Width;
	while (pos < end)
	{
		while ((pos < end) &&
			(OLED_LastDisplayBuf[Page][pos] == OLED_DisplayBuf[Page][pos]))
		{
			pos++;
		}
		if (pos >= end)
		{
			break;
		}

		runStart = pos;
		lastDirty = pos;
		cleanCount = 0U;
		pos++;

		while (pos < end)
		{
			if (OLED_LastDisplayBuf[Page][pos] != OLED_DisplayBuf[Page][pos])
			{
				lastDirty = pos;
				cleanCount = 0U;
			}
			else
			{
				cleanCount++;
				if (cleanCount >= OLED_DIRTY_SPLIT_GAP)
				{
					pos = lastDirty + 1U;
					break;
				}
			}
			pos++;
		}

		runWidth = lastDirty - runStart + 1U;
		OLED_SetCursor(Page, (uint8_t)runStart);
		OLED_WriteData(&OLED_DisplayBuf[Page][runStart], (uint8_t)runWidth);
		if (OLED_I2C_NeedInit)
		{
			OLED_LastDisplayValid = false;
			return false;
		}
		memcpy(&OLED_LastDisplayBuf[Page][runStart],
			&OLED_DisplayBuf[Page][runStart], runWidth);
	}

	return true;
}

/**
  * 函    数：OLED_Clear
  * 功    能：清空 OLED 显存缓冲区
  * 参    数：无
  * 返 回 值：无
  * 说    明：该函数在其他文件中实现，这里仅做前置声明
  */
void OLED_Clear(void);

/**
  * 函    数：OLED_ConfigDisplay
  * 功    能：配置 OLED 控制器初始化寄存器
  * 参    数：无
  * 返 回 值：无
  * 说    明：初始化和异常恢复后都会调用该函数重新配置屏幕
  */
static void OLED_ConfigDisplay(void)
{
	OLED_WriteCommand(0xAE);
	OLED_WriteCommand(0xD5);
	OLED_WriteCommand(0xf0);
	OLED_WriteCommand(0xA8);
	OLED_WriteCommand(0x3F);
	OLED_WriteCommand(0xD3);
	OLED_WriteCommand(0x00);
	OLED_WriteCommand(0x40);
	OLED_WriteCommand(0xA1);
	OLED_WriteCommand(0xC8);
	OLED_WriteCommand(0xDA);
	OLED_WriteCommand(0x12);
	OLED_WriteCommand(0x81);
	OLED_WriteCommand(0xDF);
	OLED_WriteCommand(0xD9);
	OLED_WriteCommand(0xF1);
	OLED_WriteCommand(0xDB);
	OLED_WriteCommand(0x30);
	OLED_WriteCommand(0xA4);
	OLED_WriteCommand(0xA6);
	OLED_WriteCommand(0x8D);
	OLED_WriteCommand(0x14);
	OLED_WriteCommand(0xAF);
}

/**
  * 函    数：OLED_RecoverDisplayIfNeeded
  * 功    能：根据 I2C 异常标志恢复 OLED 显示
  * 参    数：无
  * 返 回 值：无
  * 说    明：I2C 发送失败后会置位 OLED_I2C_NeedInit，本函数负责恢复总线并重配屏幕
  */
static void OLED_RecoverDisplayIfNeeded(void)
{
	if (!OLED_I2C_NeedInit)
	{
		return;
	}

	OLED_I2C_NeedInit = false;
	OLED_LastDisplayValid = false;
	(void)OLED_BusRecover();
	OLED_ConfigDisplay();
}

/**
  * 函    数：OLED初始化
  * 功    能：初始化 OLED 屏幕和底层通信接口
  * 参    数：无
  * 返 回 值：无
  * 说    明：使用前，需要调用此初始化函数
  */
void OLED_Init(void)
{
	OLED_GPIO_Init();
	OLED_ConfigDisplay();

	OLED_Clear();
	OLED_Update();
}

/**
  * 函    数：将OLED显存数组更新到OLED屏幕
  * 功    能：刷新整个 OLED 显存到屏幕
  * 参    数：无
  * 返 回 值：无
  * 说    明：所有的显示函数，都只是对OLED显存数组进行读写
  *           随后调用OLED_Update函数或OLED_UpdateArea函数
  *           才会将显存数组的数据发送到OLED硬件，进行显示
  *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Update(void)
{
	uint8_t j;

	OLED_RecoverDisplayIfNeeded();
	for (j = 0U; j < OLED_PAGE_NUM; j++)
	{
		if (!OLED_FlushDirtyRange(j, 0U, OLED_WIDTH))
		{
			return;
		}
	}
	OLED_LastDisplayValid = true;
}


/**
  * 函    数：OLED设置亮度
  * 功    能：设置 OLED 对比度/亮度
  * 参    数：Brightness ，0-255，不同显示芯片效果可能不相同。
  * 返 回 值：无
  * 说    明：不要设置过大或者过小。
  */
void OLED_Brightness(int16_t Brightness)
{
	if (Brightness > OLED_BRIGHTNESS_MAX)
	{
		Brightness = OLED_BRIGHTNESS_MAX;
	}
	if (Brightness < OLED_BRIGHTNESS_MIN)
	{
		Brightness = OLED_BRIGHTNESS_MIN;
	}
	OLED_WriteCommand(0x81);
	OLED_WriteCommand(Brightness);
}

/**
  * 函    数：将OLED显存数组部分更新到OLED屏幕
  * 功    能：刷新 OLED 显存的指定区域到屏幕
  * 参    数：X 指定区域左上角的横坐标，范围：0~OLED_WIDTH-1
  * 参    数：Y 指定区域左上角的纵坐标，范围：0~OLED_HEIGHT-1
  * 参    数：Width 指定区域的宽度，范围：0~OLED_WIDTH
  * 参    数：Height 指定区域的高度，范围：0~OLED_HEIGHT
  * 返 回 值：无
  * 说    明：此函数会至少更新参数指定的区域
  *           如果更新区域Y轴只包含部分页，则同一页的剩余部分会跟随一起更新
  * 说    明：所有的显示函数，都只是对OLED显存数组进行读写
  *           随后调用OLED_Update函数或OLED_UpdateArea函数
  *           才会将显存数组的数据发送到OLED硬件，进行显示
  *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
	uint8_t j;

	OLED_RecoverDisplayIfNeeded();

	if (X > OLED_WIDTH - 1U)
	{
		return;
	}
	if (Y > OLED_HEIGHT - 1U)
	{
		return;
	}
	if (X + Width > OLED_WIDTH)
	{
		Width = OLED_WIDTH - X;
	}
	if (Y + Height > OLED_HEIGHT)
	{
		Height = OLED_HEIGHT - Y;
	}

	for (j = Y / OLED_PAGE_HEIGHT;
		j < (Y + Height - 1U) / OLED_PAGE_HEIGHT + 1U; j++)
	{
		if (!OLED_FlushDirtyRange(j, X, Width))
		{
			break;
		}
	}
}
