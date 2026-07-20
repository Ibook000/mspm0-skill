/**
  ******************************************************************************
  * @file    OLED_driver.h
  * @brief   OLED 底层驱动接口
  ******************************************************************************
  */

#ifndef OLED_DRIVER_H
#define OLED_DRIVER_H

/*============================ 平台配置区 ============================*/

/* 新芯片移植时，将这里改成对应的平台/配置头文件 */
#ifndef OLED_DRIVER_PLATFORM_HEADER
#define OLED_DRIVER_PLATFORM_HEADER "ti_msp_dl_config.h"  /* 默认 MSPM0 SysConfig 配置头文件 */
#endif
#include OLED_DRIVER_PLATFORM_HEADER

/*
 * OLED 总线方式选择：
 * OLED_BUS_BACKEND_MSPM0_HW_I2C ：MSPM0G3519 PA0/PA1 硬件 I2C
 * OLED_BUS_BACKEND_SOFTWARE_I2C ：软件模拟 I2C，仍需在 OLED_driver.c 中适配 GPIO
 */
#define OLED_BUS_BACKEND_MSPM0_HW_I2C  1U                  /* 使用 MSPM0G3519 硬件 I2C */
#define OLED_BUS_BACKEND_SOFTWARE_I2C  2U                  /* 使用 GPIO 模拟 I2C */

#ifndef OLED_DRIVER_BUS_BACKEND
#ifdef OLED_DRIVER_USE_MSPM0_HW_I2C
#if OLED_DRIVER_USE_MSPM0_HW_I2C
#define OLED_DRIVER_BUS_BACKEND OLED_BUS_BACKEND_MSPM0_HW_I2C /* 兼容旧开关，启用硬件 I2C */
#else
#define OLED_DRIVER_BUS_BACKEND OLED_BUS_BACKEND_SOFTWARE_I2C /* 兼容旧开关，关闭硬件 I2C */
#endif
#else
#define OLED_DRIVER_BUS_BACKEND OLED_BUS_BACKEND_MSPM0_HW_I2C /* 默认使用已验证的硬件 I2C */
#endif
#endif

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

/**
  * 函    数：OLED_Init
  * 功    能：初始化 OLED 屏幕和底层通信接口
  * 参    数：无
  * 返 回 值：无
  */
void OLED_Init(void);

/**
  * 函    数：OLED_SetColorMode
  * 功    能：设置 OLED 显示颜色模式
  * 参    数：colormode，true：正常模式；false：反色数据模式
  * 返 回 值：无
  */
void OLED_SetColorMode(bool colormode);

/**
  * 函    数：OLED_Update
  * 功    能：刷新整个 OLED 显存到屏幕
  * 参    数：无
  * 返 回 值：无
  */
void OLED_Update(void);

/**
  * 函    数：OLED_UpdateArea
  * 功    能：刷新 OLED 显存的指定区域到屏幕
  * 参    数：X 指定区域左上角的横坐标
  * 参    数：Y 指定区域左上角的纵坐标
  * 参    数：Width 指定区域的宽度
  * 参    数：Height 指定区域的高度
  * 返 回 值：无
  */
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);

/**
  * 函    数：OLED_Brightness
  * 功    能：设置 OLED 对比度/亮度
  * 参    数：Brightness，范围 0~255
  * 返 回 值：无
  */
void OLED_Brightness(int16_t Brightness);

#endif /* OLED_DRIVER_H */

