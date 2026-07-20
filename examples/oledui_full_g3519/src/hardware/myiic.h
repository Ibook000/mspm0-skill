#ifndef __MYIIC_H
#define __MYIIC_H
#include "ti_msp_dl_config.h"

//设置SDA输出模式
#define SDA_OUT()   {                                                \
                        DL_GPIO_initDigitalOutput(OLED_SDA_IOMUX);    \
                        DL_GPIO_setPins(OLED_PORT, OLED_SDA_PIN);      \
                        DL_GPIO_enableOutput(OLED_PORT, OLED_SDA_PIN); \
                    }
//设置SDA输入模式
#define SDA_IN()    { DL_GPIO_initDigitalInput(OLED_SDA_IOMUX); }

//获取SDA引脚的电平变化
#define SDA_GET()   ( ( ( DL_GPIO_readPins(OLED_PORT,OLED_SDA_PIN) & OLED_SDA_PIN ) > 0 ) ? 1 : 0 )
//SDA与SCL输出
#define IIC_SDA(x)      ( (x) ? (DL_GPIO_setPins(OLED_PORT,OLED_SDA_PIN)) : (DL_GPIO_clearPins(OLED_PORT,OLED_SDA_PIN)) )
#define IIC_SCL(x)      ( (x) ? (DL_GPIO_setPins(OLED_PORT,OLED_SCL_PIN)) : (DL_GPIO_clearPins(OLED_PORT,OLED_SCL_PIN)) )


/* 硬件 I2C 操作（使用 I2C0，与 OLED 共享总线） */
uint8_t IIC_HW_WriteReg(uint8_t dev_addr, uint8_t reg_addr,
	const uint8_t *data, uint16_t len);
uint8_t IIC_HW_ReadReg(uint8_t dev_addr, uint8_t reg_addr,
	uint8_t *data, uint16_t len);

/* 软件 I2C 操作（保留兼容，新代码应使用硬件 I2C） */
void IIC_Start(void);
void IIC_Stop(void);
void IIC_Send_Byte(unsigned char txd);
unsigned char IIC_Read_Byte(unsigned char ack);
unsigned char IIC_Wait_Ack(void);
void IIC_Ack(void);
void IIC_NAck(void);

#endif
