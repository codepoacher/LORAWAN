/*!
 * \file      main.c
 *
 * \brief     Radio sensitivity test
 *
 * \remark    When LED1 stops blinking LoRa packets aren't received any more and
 *            the sensitivity level has been reached.
 *            By reading the RF generator output power we can estimate the board
 *            sensitivity
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include "board.h"
#include "gpio.h"
#include "timer.h"
#include "radio.h"
#include "Commissioning.h"

#include "frtos_app.h"
#include "frtos_mem.h"
#include "frtos_errno.h"
#include "frtos_lock.h"
#include "config_user.h"
#include "config_driver.h"
#include "frtos_ioctl.h"
#include "gpio_driver.h"
#include "frtos_sys.h"
#include "frtos_utils.h"
#include "data.pb-c.h"
#include "frtos_delay.h"
#include "task.h"
#include "frtos_drivers.h"
#include "stm32l0xx_hal.h"
#include "proto.h"
#include "lora_user.h"
#include "update.h"
#include "e2prom.h"

#if MAIN_RX
#if defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 // Hz

#elif defined( REGION_AU915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_CN470 )

#define RF_FREQUENCY                                470000000 // Hz

#elif defined( REGION_CN779 )

#define RF_FREQUENCY                                779000000 // Hz

#elif defined( REGION_EU433 )

#define RF_FREQUENCY                                433000000 // Hz

#elif defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( REGION_KR920 )

#define RF_FREQUENCY                                920000000 // Hz

#elif defined( REGION_IN865 )

#define RF_FREQUENCY                                865000000 // Hz

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_RU864 )

#define RF_FREQUENCY                                864000000 // Hz

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define USE_MODEM_LORA 
#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       10        // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_DATARATE                                50000     // bps

#if defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )

#define FSK_BANDWIDTH                               50000     // Hz >> SSB in sx127x
#define FSK_AFC_BANDWIDTH                           83333     // Hz

#elif defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )

#define FSK_BANDWIDTH                               100000    // Hz >> DSB in sx126x
#define FSK_AFC_BANDWIDTH                           166666    // Hz >> Unused in sx126x

#else
    #error "Please define a mbed shield in the compiler options."
#endif

#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * Main application entry point.
 */

static uint8_t g_pbuf[256];
static uint8_t g_size = 0;
static uint8_t g_rssi;
static uint8_t g_snr;

static int lorawan_rx( void )
{
    // Target board initialization
    BoardInitMcu( );
    BoardInitPeriph( );

    // Radio initialization
    RadioEvents.RxDone = OnRxDone;

    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );
	debug("Frequency(%dHz)\r\n "    , SX1276LoRaGetRFFrequency());

#if defined( USE_MODEM_LORA )
/*
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
*/
	uint8_t bandwidth = 0;
	uint8_t datarate =  10;
	uint8_t timeout =  5;
	Radio.SetRxConfig( MODEM_LORA, bandwidth, datarate, 1, 0, 8, timeout, false, 0, true, 0, 0, false, true );
#elif defined( USE_MODEM_FSK )

    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0, false, true );

#else
    #error "Please define a frequency band in the compiler options."
#endif

    Radio.Rx( 0 ); // Continuous Rx

    while( 1 )
    {
		//debug("%s, line = %d\r\n",__FUNCTION__, __LINE__);
        BoardLowPowerHandler( );
        // Process Radio IRQ
        if( Radio.IrqProcess != NULL )
        {
            Radio.IrqProcess( );	
        }
		if(g_size > 0)
		{
			debug("rssi:%d, snr:%d\r\n",g_rssi,g_snr);
			lwdump_data(g_pbuf, g_size);
			g_size = 0;
		}
    }
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	memcpy(g_pbuf, payload, size);
	g_size = size;
	g_rssi = rssi;
	g_snr  = snr;
}


static __const struct task lorawanrx = {
	.idx   = LORAWAN_RX_PID,
	.name  = "lorawan_rx",
	.pro   = 3,
	.depth = 700,
	.main  = lorawan_rx,
};

//TASK_REGISTER(lorawanrx);
#endif


