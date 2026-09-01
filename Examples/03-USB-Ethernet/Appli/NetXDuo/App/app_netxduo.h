/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.h
  * @author  MCD Application Team
  * @brief   NetXDuo applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_NETXDUO_H__
#define __APP_NETXDUO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "nx_api.h"

/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "ux_api.h"
#include "nxd_dhcp_client.h"
#include "ux_network_driver.h"

/* Exported macro ------------------------------------------------------------*/

/* Packet payload size */
#define PACKET_PAYLOAD_SIZE    1536

/* Packet pool size */
#define NX_PACKET_POOL_SIZE    ((1536 + sizeof(NX_PACKET)) * 60)

#define PRINT_ADDRESS(addr)    do { \
                                    printf("%s: %lu.%lu.%lu.%lu \n", #addr, \
                                    (addr >> 24) & 0xff, \
                                    (addr >> 16) & 0xff, \
                                    (addr >> 8) & 0xff, \
                                     addr& 0xff);\
                                  }while(0)

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_NetXDuo_Init(VOID *memory_ptr);
VOID nx_server_thread_entry(ULONG thread_input);

#ifdef __cplusplus
}
#endif
#endif /* __APP_NETXDUO_H__ */
