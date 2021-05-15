/**
  ******************************************************************************
  * @file           : __ROBOCON_H.h
  * @brief          : 所有的用户添加的比赛函数头文件都放到这儿
  *                   使用时只需要包含此文件即可
  ******************************************************************************
  ******************************************************************************
  */

#ifndef __ROBOCON_H
#define __ROBOCON_H

/* 系统 ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "main.h"
#include "stdlib.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include "task.h"

/* 外设 ------------------------------------------------------------------*/
#include "can.h"
#include "dma.h"
#include "gpio.h"
#include "math.h"
#include "mytype.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

/* APP ------------------------------------------------------------------*/
#include "combinations.h"
#include "jump.h"
#include "power_ctrl.h"

/* TASK ------------------------------------------------------------------*/
#include "Remote_Control.h"
#include "debug.h"
#include "detect_task.h"
#include "logicalflow_task.h"
#include "moto_ctrl.h"
#include "navigation.h"
#include "posture_ctrl.h"
#include "remote_control_task.h"

/* AHRS ------------------------------------------------------------------*/
#include "imu.h"
#include "mti30.h"

/* HARDWARE ------------------------------------------------------------------*/
#include "buzzer.h"
#include "led.h"
#include "servo.h"
#include "stepmotor.h"

/* BSP ------------------------------------------------------------------*/
#include "bsp_can.h"
#include "bsp_rc.h"
#include "bsp_uart.h"
#include "pid.h"
#include "robomoudle.h"
#include "vcan.h"

#endif /* __ROBOCON_H */
