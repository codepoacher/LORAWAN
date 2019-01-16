/*!
 * \file      main.c
 *
 * \brief     Tx Continuous Wave implementation
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

#if MAIN_TX
#if defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 // Hz
#define TX_OUTPUT_POWER                             14        // 14 dBm

#elif defined( REGION_AU915 )

#define RF_FREQUENCY                                915000000 // Hz
#define TX_OUTPUT_POWER                             14        // 14 dBm

#elif defined( REGION_CN470 )

#define RF_FREQUENCY                                470000000 // Hz
#define TX_OUTPUT_POWER                             20        // 20 dBm

#elif defined( REGION_CN779 )

#define RF_FREQUENCY                                779000000 // Hz
#define TX_OUTPUT_POWER                             14        // 14 dBm

#elif defined( REGION_EU433 )

#define RF_FREQUENCY                                433000000 // Hz
#define TX_OUTPUT_POWER                             20        // 20 dBm

#elif defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                             14        // 14 dBm

#elif defined( REGION_KR920 )

#define RF_FREQUENCY                                920000000 // Hz
#define TX_OUTPUT_POWER                             14        // 14 dBm

#elif defined( REGION_IN865 )

#define RF_FREQUENCY                                865000000 // Hz
#define TX_OUTPUT_POWER                             14        // 14 dBm

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 // Hz
#define TX_OUTPUT_POWER                             14        // 14 dBm

#elif defined( REGION_RU864 )

#define RF_FREQUENCY                                864000000 // Hz
#define TX_OUTPUT_POWER                             14        // 14 dBm

#else

    #error "Please define a frequency band in the compiler options."

#endif
#define TX_TIMEOUT                                  10        // seconds (MAX value)

static TimerEvent_t Led1Timer;
volatile bool Led1TimerEvent = false;

static TimerEvent_t Led2Timer;
volatile bool Led2TimerEvent = false;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;

/*!
 * \brief Function executed on Led 1 Timeout event
 */
void OnLed1TimerEvent( void )
{
    Led1TimerEvent = true;
}

/*!
 * \brief Function executed on Led 2 Timeout event
 */
void OnLed2TimerEvent( void )
{
    Led2TimerEvent = true;
}

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnRadioTxTimeout( void )
{
    // Restarts continuous wave transmission when timeout expires
    Radio.SetTxContinuousWave( RF_FREQUENCY, TX_OUTPUT_POWER, TX_TIMEOUT );
}

static int check_spi()
{
	uint8_t Readbuffer[3]={0x7c,0x80,0x01};
	//SX1276Reset();
	SX1276WriteBuffer( 0x06, Readbuffer, sizeof(Readbuffer) );
	Readbuffer[0] = Readbuffer[1] = Readbuffer[2] = 0;
	//SX1276ReadBuffer( REG_LR_FRFMSB, Readbuffer,2 );
	SX1276ReadBuffer( 0x06, Readbuffer,sizeof(Readbuffer) );
	if((Readbuffer[0] != 0x7C) || (Readbuffer[1] != 0x80) || (Readbuffer[2] != 0x01))
	{
		debug("Readbuffer:%x,%x,%x\r\n",Readbuffer[0],Readbuffer[1],Readbuffer[2]);
		debug("spi check error1\r\n");
		return -1;
	}
	return 0;
}
/**
 * Main application entry point.
 */
static int lorawan_tx( void )
{
	uint8_t buff[10] = {1,2,4,4,5,6,7,8,9,0};
    // Target board initialization
	BoardInitMcu( );
    //BoardInitPeriph( );
	SX1276Reset(  );
	vTaskDelay(10);

	debug("%s, line = %d\r\n",__FUNCTION__, __LINE__);
    TimerInit( &Led1Timer, OnLed1TimerEvent );
    TimerSetValue( &Led1Timer, 3000 );

    TimerInit( &Led2Timer, OnLed2TimerEvent );
    TimerSetValue( &Led2Timer, 5000 );
	
    // Switch LED 1 ON
    //GpioWrite( &Led1, 0 );
    TimerStart( &Led1Timer );
	
	if(check_spi() == -1)
		return -1;

    // Radio initialization
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    //Radio.SetTxContinuousWave( RF_FREQUENCY, TX_OUTPUT_POWER, TX_TIMEOUT );
	uint8_t bandwidth = 0;
	uint8_t datarate =  10;
	Radio.SetTxConfig( MODEM_LORA, 20, 0, bandwidth, datarate, 1, 8, false, true, 0, 0, false, 3000 );

	Radio.SetMaxPayloadLength( MODEM_LORA, 255);
	Radio.SetModem( MODEM_LORA );
	//SX1276Write(0x39,0x34);//设置同步字的值
	
	debug("Frequency(%dHz)\r\n "    , SX1276LoRaGetRFFrequency());
	
    // Blink LEDs just to show some activity
    while( 1 )
    {
        // Process Radio IRQ
        if( Radio.IrqProcess != NULL )
        {
            Radio.IrqProcess( );
        }
        if( Led1TimerEvent == true )
        {
            Led1TimerEvent = false;

			Radio.Send(buff,10);
            //debug("led1\r\n");
			lwrtc_time_print();
            TimerStart( &Led1Timer );
        }
    }
}

static __const struct task lorawantx = {
	.idx   = LORAWAN_TX_PID,
	.name  = "lorawan_tx",
	.pro   = 3,
	.depth = 700,
	.main  = lorawan_tx,
};

TASK_REGISTER(lorawantx);
#endif
