/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
// Modified by Adriana Arizaga
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/opt.h"
#include "aes.h"

#if LWIP_NETCONN

#include "tcpecho.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"

#include "board.h"

#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "aes.h"
#include "fsl_crc.h"

#include "aaj_layer.c"
#include "aaj_layer.h"
#include "aaj_layer_interface.h"
#include "aaj_layer_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

//IP address configuration.
#define configIP_ADDR0 192
#define configIP_ADDR1 168
#define configIP_ADDR2 0
#define configIP_ADDR3 102

#include "aes.h"

/* Netmask configuration. */
#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 255
#define configNET_MASK3 0

//Gateway address configuration.
#define configGW_ADDR0 192
#define configGW_ADDR1 168
#define configGW_ADDR2 0
#define configGW_ADDR3 100

// MAC del micro IP micro ejem. 10.215.170.112
/* MAC address configuration. */
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

/* System clock name. */
#define EXAMPLE_CLOCK_NAME kCLOCK_CoreSysClk


#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

/*!
 * @brief Main function
 */
int main(void)
{
    static struct netif netif;
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
    static mem_range_t non_dma_memory[] = NON_DMA_MEMORY_ARRAY;
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyAddress = EXAMPLE_PHY_ADDRESS,
        .clockName  = EXAMPLE_CLOCK_NAME,
        .macAddress = configMAC_ADDR,
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
        .non_dma_memory = non_dma_memory,
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    };

    SYSMPU_Type *base = SYSMPU;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    /* Disable SYSMPU. */
    base->CESR &= ~SYSMPU_CESR_VLD_MASK;

    IP4_ADDR(&netif_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3); // toma los 4 octetos de configIP_ADDR y los mete a la direccion IP de 32 bits
    IP4_ADDR(&netif_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&netif_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    tcpip_init(NULL, NULL);

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,// configura las direcciones
                       tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" TCP Echo example\r\n");
    PRINTF("************************************************\r\n");
    PRINTF(" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_ipaddr)[0], ((u8_t *)&netif_ipaddr)[1],
           ((u8_t *)&netif_ipaddr)[2], ((u8_t *)&netif_ipaddr)[3]);
    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&netif_netmask)[0], ((u8_t *)&netif_netmask)[1],
           ((u8_t *)&netif_netmask)[2], ((u8_t *)&netif_netmask)[3]);
    PRINTF(" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_gw)[0], ((u8_t *)&netif_gw)[1],
           ((u8_t *)&netif_gw)[2], ((u8_t *)&netif_gw)[3]);
    PRINTF("************************************************\r\n");

    tcpecho_init();

    sys_thread_new("aescrc_task", aescrc_test_task, NULL, 1024, 4);

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
#endif

static void InitCrc32(CRC_Type *base, uint32_t seed);  //  aaj_layer.c


void aescrc_test_task(void *arg)  // ejemplo de encripcion por aes
{
	CRC_Type *base = CRC0;
	uint32_t checksum32;

	uint8_t crc[4] = {0};
	uint8_t message[16]= {0}; // ejem. 16 bytes


	//d5 e6 d9 27 d7 fb 01 e7 7e 21 f8 89 9e 68 3a a0    /// ejemp. mensaje cifrado enviado x cliente 16 bytes
	uint8_t test[] = { 0xd5, 0xe6, 0xd9, 0x27, 0xd7, 0xfb, 0x01, 0xe7, 0x7e, 0x21, 0xf8, 0x89, 0x9e, 0x68, 0x3a, 0xa0};

	size_t test_len = sizeof(test); //16
	for(int i=0; i<sizeof(test); i++) // muestra mensaje
	{
		PRINTF("0x%02x,", test[i]);
	}
	PRINTF("\r\n");
	//2b 7e 15 16 28 ae d2 a6 ab f7 15 88 09 cf 4f 3c ejemplo key
	uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
	//uint8_t iv[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	//uint8_t out[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	struct AES_ctx ctx;

	PRINTF("AES_init_ctx_iv\r\n");
	//AES_init_ctx_iv(&ctx, key, iv);
	AES_init_ctx(&ctx, key);
	PRINTF("AES_CBC_decrypt_buffer\r\n");
	AES_ECB_decrypt(&ctx, test); //000b4500010203040506070884022bb1
	PRINTF("decrypt_buffer\r\n");
	//AES_CBC_decrypt_buffer(&ctx, test[0], test_len);
	for(int i=0; i<sizeof(test); i++)
	{
		PRINTF("0x%02x,", test[i]);
	}

	memcpy(message, test,test_len-4);  // copia datos de test a mensaje 12 bytes
	for(int i=0; i<sizeof(message); i++) // muestra mensaje
	{
		PRINTF("0x%02x,", message[i]);
	}
	PRINTF("\r\n");
	PRINTF("InitCrc32\r\n");
    InitCrc32(base, 0xFFFFFFFFU); // inicializa CRC
    CRC_WriteData(base, message, sizeof(message));
    checksum32 = CRC_Get32bitResult(base); //84022bb1 genera el CRC
    PRINTF("CRC-32: 0x%08x\r\n", checksum32);
    // comparara checsum32
	/* AES data */
	// usa esa llave de encripcion key y el vector de inicializacion iv[]

}
