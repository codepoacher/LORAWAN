/*!
 * \file      board-config.h
 *
 * \brief     Board configuration
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
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */
#define BOARD_TCXO_WAKEUP_TIME                      0

/*!
 * Board MCU pins definitions
 */

/*dio*/
#define SX127X1_RST_GPIO    E
#define SX127X1_RST_PIN		7

#define SX127X1_TX_GPIO     D
#define SX127X1_TX_PIN		3

#define SX127X1_RX_GPIO     D
#define SX127X1_RX_PIN		2

#define SX127X1_DIO0_GPIO	E
#define SX127X1_DIO0_PIN	5

#define SX127X1_DIO1_GPIO	C
#define SX127X1_DIO1_PIN	13

#define SX127X1_DIO2_GPIO	E
#define SX127X1_DIO2_PIN	6

#define SX127X1_DIO3_GPIO	H
#define SX127X1_DIO3_PIN	10

#define SX127X1_DIO4_GPIO	B
#define SX127X1_DIO4_PIN	2

#define SX127X1_DIO5_GPIO	H
#define SX127X1_DIO5_PIN	9


#define RADIO_RESET                                 GPIO( E, 7  ) 

#define RADIO_DIO_0                                 GPIO( E, 5  )
#define RADIO_DIO_1                                 GPIO( C, 13 )
#define RADIO_DIO_2                                 GPIO( E, 6  )
#define RADIO_DIO_3                                 GPIO( H, 10 )
#define RADIO_DIO_4                                 GPIO( B, 2  )
#define RADIO_DIO_5                                 GPIO( H, 9  )

#define LED_1                                       GPIO(  D, 10 )

//#define UART_TX                                     GPIO( GPIO_PORTA, 22 )
//#define UART_RX                                     GPIO( GPIO_PORTA, 23 )

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            GPIO( D,  10 )
#define RADIO_DBG_PIN_RX                            GPIO( D,  10 )

#define RTC_DBG_PIN_0                               GPIO( D, 10 )
#define RTC_DBG_PIN_1                               GPIO( D, 11 )

#define HWTMR_DBG_PIN_0                             GPIO( D, 30 )

#endif // __BOARD_CONFIG_H__
