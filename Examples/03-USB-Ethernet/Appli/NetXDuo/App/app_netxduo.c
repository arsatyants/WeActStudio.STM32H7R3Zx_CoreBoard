/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_netxduo.h"

/* Private define ------------------------------------------------------------*/
#define NULL_IP_ADDRESS      IP_ADDRESS(0,0,0,0)

/* Private variables ---------------------------------------------------------*/
TX_THREAD          NxApp_thread;
TX_SEMAPHORE       WebServer_Semaphore;
NX_PACKET_POOL     net_packet_pool;
NX_IP              cdc_ecm_ip;
NX_DHCP            dhcp_client;

/* Private function prototypes -----------------------------------------------*/

/* DHCP state change notify callback */
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr);

/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;
  UCHAR *pointer;

  /* Initialize the NetXDuo system */
  nx_system_initialize();

  /* Allocate stack for the application thread */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 1024, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create the application thread (DHCP + IP reporting) */
  if (tx_thread_create(&NxApp_thread, "NetXDuo App thread", nx_server_thread_entry, 0,
                       pointer, 1024, 10, 10, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Allocate stack for the packet pool */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, NX_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create a packet pool */
  if (nx_packet_pool_create(&net_packet_pool, "NetX Main Packet Pool",
                            PACKET_PAYLOAD_SIZE, pointer,
                            NX_PACKET_POOL_SIZE) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Allocate stack for the IP instance */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 2048, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Creates an CDC ECM Internet Protocol instance */
  if (nx_ip_create(&cdc_ecm_ip, "NetX IP Instance 0", NULL_IP_ADDRESS,
                   NULL_IP_ADDRESS, &net_packet_pool, _ux_network_driver_entry,
                   pointer, 2048, 10) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Allocate stack for ARP */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 2048, TX_NO_WAIT) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable ARP and supply ARP cache memory for IP Instance CDC ECM */
  if (nx_arp_enable(&cdc_ecm_ip, (void *) pointer, 1024) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable TCP traffic */
  if (nx_tcp_enable(&cdc_ecm_ip) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable UDP traffic */
  if (nx_udp_enable(&cdc_ecm_ip) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Enable ICMP to enable the ping utility */
  if (nx_icmp_enable(&cdc_ecm_ip) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create the DHCP Client instance */
  if (nx_dhcp_create(&dhcp_client, &cdc_ecm_ip, "dhcp_client") != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Register a callback function for ip notify */
  if (nx_ip_address_change_notify(&cdc_ecm_ip, ip_address_change_notify_callback,
                                  NULL) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Create WebServer semaphore (reused here as "IP assigned" semaphore) */
  if (tx_semaphore_create(&WebServer_Semaphore, "IP Assigned Semaphore", 0) != TX_SUCCESS)
  {
    Error_Handler();
  }

  return ret;
}

/**
  * @brief  ip_address_change_notify_callback
            IP change notification function
  * @param  ip_instance : NX_IP instance registered for this callback
  * @param  ptr : optional data pointer
  * @retval none
  */
static VOID ip_address_change_notify_callback(NX_IP *ip_instance, VOID *ptr)
{
  tx_semaphore_put(&WebServer_Semaphore);
}

/**
  * @brief  nx_server_thread_entry
  *         Application thread: brings up DHCP and prints the assigned IP.
  * @param  thread_input : thread input
  * @retval None
  */
void nx_server_thread_entry(ULONG thread_input)
{
  ULONG IPAddress;
  ULONG NetMask;

  NX_PARAMETER_NOT_USED(thread_input);

  printf("Starting DHCP client...\n");

  /* Start the DHCP Client */
  if (nx_dhcp_start(&dhcp_client) != NX_SUCCESS)
  {
    Error_Handler();
  }

  /* Wait until an IP address has been assigned */
  if (tx_semaphore_get(&WebServer_Semaphore, TX_WAIT_FOREVER) != TX_SUCCESS)
  {
    Error_Handler();
  }

  /* Checks for errors in the IP address */
  nx_ip_address_get(&cdc_ecm_ip, &IPAddress, &NetMask);

  /* Log IP address */
  PRINT_ADDRESS(IPAddress);

  /* Log NetMask */
  PRINT_ADDRESS(NetMask);
}
