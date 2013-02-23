#ifndef _DEV_API_H_
#define _DEV_API_H_
/******************************************************************************/
/*                                                                            */
/*             Copyright (c) 1999, 2000 by RealChip Inc.                      */
/*                                                                            */
/*  All rights are reserved.  Reproduction in whole or in part is prohibited  */
/*  without the written consent of the copyright owner.                       */
/*                                                                            */
/*  RealChip Inc. reserves the right to make changes without notice  at  any  */
/*  time. RealChip Inc. makes no warranty, expressed, implied  or  statutory, */
/*  including but not limited to any implied warranty  of  merchantability of */
/*  fitness for any particular purpose, or that the use will not infringe any */
/*  third party patent, copyright or trademark. RealChip Inc. must not be     */
/*  liable for any loss or damage arising from its use.                       */
/*                                                                            */
/*  This Copyright notice may not be removed or modified without prior        */
/*  written consent of RealChip Inc.                                          */
/*                                                                            */
/******************************************************************************/
//***********************************************************************************
//*  COPYRIGHT (C) 2002 TEXAS INSTRUMENTS INCORPORATED.
//*  ALL RIGHTS RESERVED. ONLY TO BE USED ON TEXAS INSTRUMENTS SILICON.
//***********************************************************************************
//*
//*   FILE:           DevFwApi.h
//*   LANGUAGE:       ANSI C
//*   SUBSYSTEM:      
//*   AUTHOR:         AG
//*   CREATION DATE:  09/20/99
//*
//*   ABSTRACT:       TI TUSB5152 (UMP) project.
//*                   UMP constants, type definitions, prototypes, etc...
//*                   Details can be found in following documents:
//*                   -  TIUMP/API-001
//*                   -  TIUMP/API-002
//*
//* developed for:	Texas Instruments.
//*
//* REVISION HISTORY:
//*
//*
//*   Date         By       Description
//*   -----------  ---  --------------------------------------------------------
//*   Mar-30-1999  IVK  Created
//*   Apr-09-1999  IVK  Modified accordingly TIUMP/API-001 rev 0.30
//*   Apr-10-1999  NMD  Modified accordingly TIUMP/API-002 rev 0.30
//*   Apr-15-1999  NMD  Fixed bugs in DMA cfg data & UART read/write structs
//*   Apr-16-1999  IVK  Modified accordingly TIUMP/API-001 rev 0.40
//*   Apr-22-1999  LT   Add error codes for boot code and may be more
//*   Apr-16-1999  IVK  Modified accordingly TIUMP/API-001 rev 0.41
//*   Apr-30-1999  IVK  Modified accordingly TIUMP/API-001 rev 0.51
//*   May-04-1999  IVK  Modified accordingly TIUMP/API-001 rev 0.52
//*   May-14-1999  IVK  IEEE-1284 ECP and EPP mode constants' values 
//*                     swapped (now correspond to TUSB5152 enums).
//*   Jan-06-2000  NMD  Made changes to accomodate the changes 
//*                     to put new device Frame work for UMP.
//*   Jan-21-2000  NMD  UMP code release 0.51 done with new DFW.
//*   Feb-04-2000  NMD  UMP code release 0.52 done with new DFW.
//*   Mar-04-2000  IVK  Release 0.94 done and debugged.
//*   Apr-12-2000  IVK  Nothing was changed. Released under number 1.00
//*
//*
//*  STR #   INIT   DATE            SHORT TITLE
//* [00174]  DWB  21 NOV 02 Update headers..........................................
//* [xxxxx]  nnn  dd mmm yy ........................................................
//*
//***********************************************************************************
//*

#ifdef macintosh
	#define BYTE UInt8
	#define WORD UInt16
	#define UMP_XP_DRIVER
#else
#ifdef _HOST_
#pragma pack(push)
#pragma pack(1)
#endif
 
#ifndef _HOST_
#include "DataTypes.h"
#endif
#endif


#ifdef  __cplusplus
extern  "C"
{
#endif

/*------------------------------------------------------------------------------
  PREPROCESSOR CONSTANTS
  ----------------------------------------------------------------------------*/


/* Device firmware capabilities flags                                         */
#define DTK_HW_SUPP_MASK             0x0001 /* Device has hardware debug 
                                               registers and firmware supports 
                                               it                             */
#define DTK_RW_SUPP_MASK             0x0002 /* Firmware supports read-write
                                               registers / memory debug
                                               commands                       */

#define DTK_WUP_SUPP_MASK            0x0004 /* Firmware supports remote wakeup
                                               debug commands                 */

#define DTK_RTK_SUPP_MASK            0x0008 /* Firmware contains debug version 
                                               of real-time kernel and supports
                                               RTK debug commands             */

#define DTK_MONITOR_MASK             0x0010 /* Firmware contains monitor      */

#define DTK_USB0_SUPP_MASK           0x0020 /* Debug commands is supported via
                                               USB control channel (USB vendor 
                                               specific commands)             */ 
                                               
#define DTK_USBX_SUPP_MASK           0x0040 /* Debug commands is supported via
                                               USB bulk transfers channel (see
                                               command set description for 
                                               EP address and others details  */ 
                                               
#define DTK_SERIAL_SUPP_MASK         0x0080 /* Debug commands is supported via
                                               MSC-51 RS-232                  */ 

/* Address and data types constants                                           */

#define DTK_SIZE_NS                    0x00  /* Addr/data size isn't specified*/
#define DTK_SIZE_BYTE                  0x01  /* Addr/data size is BYTE        */
#define DTK_SIZE_WORD                  0x02  /* Addr/data size is WORD        */
#define DTK_SIZE_DWORD                 0x04  /* Addr/data size is DWORD       */
  

#define DTK_ADDR_SPACE_SFR             0x10  /* Addr is placed in SFR space   */
#define DTK_ADDR_SPACE_IDATA           0x20  /* Addr is placed in IDATA space */
#define DTK_ADDR_SPACE_XDATA           0x30  /* Addr is placed in XDATA space */
#define DTK_ADDR_SPACE_CODE            0x40  /* Addr is placed in CODE space  */ 
#define DTK_ADDR_SPACE_GPIO            0x50  /* Addr is GPIO address          */
#define DTK_ADDR_SPACE_I2C             0x60  /* Addr is placed in I2C area    */ 
#define DTK_ADDR_SPACE_FLASH           0x70  /* Addr is placed in FLASH area  */ 
#define DTK_ADDR_SPACE_DSP             0x80  /* Addr is placed in DSP regs 
                                                space                         */
                                             
#define DTK_TYPE_DATA_SEQ              0x10  /* Data is stored on the sequental
                                                addresses (like RAM)          */

#define DTK_MASK_SIZE                  0x0F
#define DTK_MASK_TYPE                  0xF0

/*----------------------------------------------------------------------------*/


/* UMP endpoints number, assigned to ports                                    */
#define UMP_UART1_EP                   0x01
#ifdef UMP_INCLUDE_UART2
#define UMP_UART2_EP                   0x02
#endif // UMP_INCLUDE_UART2
#ifdef UMP_INCLUDE_IEEE1284
#define UMP_1284_EP                    0x03
#endif // UMP_INCLUDE_IEEE1284

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* UMP USB interrupts related things                                          */
#define UMPI_EP_NUM                    0x07  /* Interrupt EP number           */
// IGX:BK 2002/01/13: UMPI_EP_NUM not used in driver, therefore not conditional
//  for TIUSB3410.  In TIUSB5152 firmware, there is confusion between Endpoint
//  Address (0x87) and Endpoint Number (index in Descriptor Block Array) which
//  is 7 in TIUSB5152; but presumably 5 in TIUSB5052 and 3 in TIUSB3410.

/* Interrupt code mask                                                        */             
#define UMPI_INT_PORT_MASK           0xF000
#define UMPI_INT_EVENT_MASK          0x0F00  

/* Interrupt ports                                                            */
#define UMPI_I2C                     0x1000
#ifdef UMP_INCLUDE_IEEE1284
#define UMPI_1284                    0x2000
#endif // UMP_INCLUDE_IEEE1284
#define UMPI_UART1                   0x3000
#ifdef UMP_INCLUDE_UART2
#define UMPI_UART2                   0x4000
#endif // UMP_INCLUDE_UART2

/* Interrupt events                                                           */ 

#ifdef UMP_INCLUDE_IEEE1284
#define UMPI_P1284_FAULT             0x0400
#define UMPI_P1284_PAPER_ERROR       0x0500
#define UMPI_P1284_ACK               0x0600
#endif // UMP_INCLUDE_IEEE1284

#define UMPI_PUART_DATA_ERROR        0x0300     /* UART specific              */
#define UMPI_PUART_MODEM_STATUS      0x0400



/* UMP USB interrupts packets    
                                             */
#ifdef UMP_INCLUDE_IEEE1284
#define UMPI_1284_FAULT                    ( UMPI_1284 | UMPI_P1284_FAULT )
                            
#define UMPI_1284_PAPER_ERROR              ( UMPI_1284 | UMPI_P1284_PAPER_ERROR )

#define UMPI_1284_ACK                      ( UMPI_1284 | UMPI_P1284_ACK         )
#endif // UMP_INCLUDE_IEEE1284
                                             
#define UMPI_UART1_DATA_ERROR              ( UMPI_UART1 |                      \
                                             UMPI_PUART_DATA_ERROR )
                                             
#define UMPI_UART1_MODEM_STATUS            ( UMPI_UART1 |                      \
                                             UMPI_PUART_MODEM_STATUS )
                                             
#ifdef UMP_INCLUDE_UART2                                             
#define UMPI_UART2_DATA_ERROR              ( UMPI_UART2 |                      \
                                             UMPI_PUART_DATA_ERROR )
                                             
#define UMPI_UART2_MODEM_STATUS            ( UMPI_UART2 |                      \
                                             UMPI_PUART_MODEM_STATUS )
#endif // UMP_INCLUDE_UART2                                            
#define UMPI_INTERNAL_ERROR                0xFFFF 

/*----------------------------------------------------------------------------*/
/* Port Status Error Codes and Masks                                          */
#ifdef UMP_INCLUDE_IEEE1284
#define UMP_MASK_IEEE1284_ERROR              0x08
#define UMP_MASK_IEEE1284_PAPER_ERROR        0x20
#define UMP_MASK_IEEE1284_READY              0x80
#endif // UMP_INCLUDE_IEEE1284

/*----------------------------------------------------------------------------*/
/* UART port test errors flags                                                */
#define UMP_MASK_UART_INT_ERR                0x01 
#define UMP_MASK_UART_EXT_ERR                0x02 

#ifdef UMP_INCLUDE_IEEE1284
/*----------------------------------------------------------------------------*/
/* IEEE-1284 port test errors flags                                           */
#define UMP_MASK_1284_DAT_ERR                0x01 /* Data register error      */
#define UMP_MASK_1284_ADR_ERR                0x02 /* Address register error   */
#define UMP_MASK_1284_CTL_ERR                0x04 /* Control register error   */
#define UMP_MASK_1284_IMS_ERR                0x08 /* Int mask register error  */
#define UMP_MASK_1284_MCR_ERR                0x10 /* Mode control reg error   */
#define UMP_MASK_1284_LBK_ERR                0x20 /* External loopback error  */

/*----------------------------------------------------------------------------*/
/* Port External Device Modes Constants                                       */
#define UMP_MASK_DEVMODE_UNDEFINED           0x00
#define UMP_MASK_DEVMODE_PNIBBLE             0x02
#define UMP_MASK_DEVMODE_PBYTE               0x04
#define UMP_MASK_DEVMODE_PEPP                0x08
#define UMP_MASK_DEVMODE_PECP                0x10
#define UMP_MASK_DEVMODE_SDEVICE             0x20

#endif // UMP_INCLUDE_IEEE1284




/*----------------------------------------------------------------------------*/
/* Config Settings Constants                                                  */
#define I2C_BUS_SPEED_100                       0
#define I2C_BUS_SPEED_400                       1 

#ifdef UMP_INCLUDE_IEEE1284
/* 1284 Settings Constants                                                    */
#define UMP_IEEE1284_SPP                     0x01
#define UMP_IEEE1284_EPP                     0x02
#define UMP_IEEE1284_ECP                     0x03
#define UMP_IEEE1284_ECP_RLE                 0x04
#define UMP_IEEE1284_NBL                     0x05
#define UMP_IEEE1284_SPP_SOFT_XFER           0x06
#define UMP_IEEE1284_SPP_SOFT_XFER_ACK       0x07

#define UMP_IEEE1284_READ                    0x80

/* bFlags constants                                                           */
#define UMP_MASK_1284_FLAGS_USE_ADDR         0x01
#endif // UMP_INCLUDE_IEEE1284

//-------------------------------------------------------------------------
// PJG: We use a different baud rate scheme for w2k driver since device 
//			type is handled dynamically at run time.
//	    We use seperate tables based on which chipset is currently running.
//-------------------------------------------------------------------------
#ifdef UMP_XP_DRIVER
#define UMP_3410_UART_50                   0x480E /* Baud rate 50             */
#define UMP_3410_UART_75                   0x3014 /* Baud rate 75             */
#define UMP_3410_UART_110                  0x20C8 /* Baud rate 110            */
#define UMP_3410_UART_135                  0x1AD0 /* Baud rate 135            */
#define UMP_3410_UART_150                  0x180A /* Baud rate 150            */
#define UMP_3410_UART_300                  0x0C05 /* Baud rate 300            */
#define UMP_3410_UART_600                  0x0602 /* Baud rate 600            */
#define UMP_3410_UART_1200                 0x0301 /* Baud rate 1200           */
#define UMP_3410_UART_1800                 0x0200 /* Baud rate 1800           */
#define UMP_3410_UART_2000                 0x01CE /* Baud rate 2000           */
#define UMP_3410_UART_2400                 0x0181 /* Baud rate 2400           */
#define UMP_3410_UART_3600                 0x0100 /* Baud rate 3600           */
#define UMP_3410_UART_4800                 0x00C0 /* Baud rate 4800           */
#define UMP_3410_UART_7200                 0x0080 /* Baud rate 7200           */
#define UMP_3410_UART_9600                 0x0060 /* Baud rate 9600           */
#define UMP_3410_UART_19K                  0x0030 /* Baud rate 19200          */
#define UMP_3410_UART_38K                  0x0018 /* Baud rate 38400          */
#define UMP_3410_UART_57K                  0x0010 /* Baud rate 57600          */
#define UMP_3410_UART_115K                 0x0008 /* Baud rate 115200         */
#define UMP_3410_UART_230K                 0x0004 /* Baud rate 230400         */
#define UMP_3410_UART_460K                 0x0002 /* Baud rate 460800         */
#define UMP_3410_UART_920K                 0x0001 /* Baud rate 921600         */

#define UMP_5x52_UART_50                   0x240F /* Baud rate 50             */
#define UMP_5x52_UART_75                   0x180A /* Baud rate 75             */
#define UMP_5x52_UART_110                  0x1064 /* Baud rate 110            */
#define UMP_5x52_UART_135                  0x0D68 /* Baud rate 135            */
#define UMP_5x52_UART_150                  0x0C05 /* Baud rate 150            */
#define UMP_5x52_UART_300                  0x0602 /* Baud rate 300            */
#define UMP_5x52_UART_600                  0x0301 /* Baud rate 600            */
#define UMP_5x52_UART_1200                 0x0181 /* Baud rate 1200           */
#define UMP_5x52_UART_1800                 0x0100 /* Baud rate 1800           */
#define UMP_5x52_UART_2000                 0x00E7 /* Baud rate 2000           */
#define UMP_5x52_UART_2400                 0x00C0 /* Baud rate 2400           */
#define UMP_5x52_UART_3600                 0x0080 /* Baud rate 3600           */
#define UMP_5x52_UART_4800                 0x0060 /* Baud rate 4800           */
#define UMP_5x52_UART_7200                 0x0040 /* Baud rate 7200           */
#define UMP_5x52_UART_9600                 0x0030 /* Baud rate 9600           */
#define UMP_5x52_UART_19K                  0x0018 /* Baud rate 19200          */
#define UMP_5x52_UART_38K                  0x000C /* Baud rate 38400          */
#define UMP_5x52_UART_57K                  0x0008 /* Baud rate 57600          */
#define UMP_5x52_UART_115K                 0x0004 /* Baud rate 115200         */
#define UMP_5x52_UART_230K                 0x0002 /* Baud rate 230400         */
#define UMP_5x52_UART_460K                 0x0001 /* Baud rate 460800         */
#endif


// Original tables used by win9x driver and still used by debug output.
// TUSB3410 support 920K baudrate
#ifdef UMP_PRODUCT_3410
#define UMP_BAUD_RATES_QTY                   0x16
/* Baud rate divisors                                                         */
#define UMP_UART_50                        0x480E /* Baud rate 50             */
#define UMP_UART_75                        0x3014 /* Baud rate 75             */
#define UMP_UART_110                       0x20C8 /* Baud rate 110            */
#define UMP_UART_135                       0x1AD0 /* Baud rate 135            */
#define UMP_UART_150                       0x180A /* Baud rate 150            */
#define UMP_UART_300                       0x0C05 /* Baud rate 300            */
#define UMP_UART_600                       0x0602 /* Baud rate 600            */
#define UMP_UART_1200                      0x0301 /* Baud rate 1200           */
#define UMP_UART_1800                      0x0200 /* Baud rate 1800           */
#define UMP_UART_2000                      0x01CE /* Baud rate 2000           */
#define UMP_UART_2400                      0x0181 /* Baud rate 2400           */
#define UMP_UART_3600                      0x0100 /* Baud rate 3600           */
#define UMP_UART_4800                      0x00C0 /* Baud rate 4800           */
#define UMP_UART_7200                      0x0080 /* Baud rate 7200           */
#define UMP_UART_9600                      0x0060 /* Baud rate 9600           */
//#define UMP_UART_144K                      0x0040 /* Baud rate 14400           */
#define UMP_UART_19K                       0x0030 /* Baud rate 19200          */
#define UMP_UART_38K                       0x0018 /* Baud rate 38400          */
#define UMP_UART_57K                       0x0010 /* Baud rate 57600          */
#define UMP_UART_115K                      0x0008 /* Baud rate 115200         */
#define UMP_UART_230K                      0x0004 /* Baud rate 230400         */
#define UMP_UART_460K                      0x0002 /* Baud rate 460800         */
#define UMP_UART_920K                      0x0001 /* Baud rate 921600         */
#else
#define UMP_BAUD_RATES_QTY                 0x15
/* Baud rate divisors                                                         */
#define UMP_UART_50                        0x240F /* Baud rate 50             */
#define UMP_UART_75                        0x180A /* Baud rate 75             */
#define UMP_UART_110                       0x1064 /* Baud rate 110            */
#define UMP_UART_135                       0x0D68 /* Baud rate 135            */
#define UMP_UART_150                       0x0C05 /* Baud rate 150            */
#define UMP_UART_300                       0x0602 /* Baud rate 300            */
#define UMP_UART_600                       0x0301 /* Baud rate 600            */
#define UMP_UART_1200                      0x0181 /* Baud rate 1200           */
#define UMP_UART_1800                      0x0100 /* Baud rate 1800           */
#define UMP_UART_2000                      0x00E7 /* Baud rate 2000           */
#define UMP_UART_2400                      0x00C0 /* Baud rate 2400           */
#define UMP_UART_3600                      0x0080 /* Baud rate 3600           */
#define UMP_UART_4800                      0x0060 /* Baud rate 4800           */
#define UMP_UART_7200                      0x0040 /* Baud rate 7200           */
#define UMP_UART_9600                      0x0030 /* Baud rate 9600           */
#define UMP_UART_19K                       0x0018 /* Baud rate 19200          */
#define UMP_UART_38K                       0x000C /* Baud rate 38400          */
#define UMP_UART_57K                       0x0008 /* Baud rate 57600          */
#define UMP_UART_115K                      0x0004 /* Baud rate 115200         */
#define UMP_UART_230K                      0x0002 /* Baud rate 230400         */
#define UMP_UART_460K                      0x0001 /* Baud rate 460800         */
#endif

/* Bits per character                                                         */
#define UMP_UART_CHAR5BITS                   0x00
#define UMP_UART_CHAR6BITS                   0x01
#define UMP_UART_CHAR7BITS                   0x02
#define UMP_UART_CHAR8BITS                   0x03

/* Parity                                                                     */
#define UMP_UART_NOPARITY                    0x00
#define UMP_UART_ODDPARITY                   0x01
#define UMP_UART_EVENPARITY                  0x02
#define UMP_UART_MARKPARITY                  0x03
#define UMP_UART_SPACEPARITY                 0x04

#define UMP_UART_PARITY_SETS_QTY             0x05


/* Stop bits                                                                  */
#define UMP_UART_STOPBIT1                    0x00
#define UMP_UART_STOPBIT15                   0x01
#define UMP_UART_STOPBIT2                    0x02

#define UMP_UART_232MODE                     0x00
#define UMP_UART_485MODE_RD                  0x01  /* Receiver disable        */
#define UMP_UART_485MODE_RE                  0x02  /* Receiver enable         */


/*----------------------------------------------------------------------------*/
/* Port Settings Constants  (continued)                                       */
/* wFlags constants                                                           */
#define UMP_MASK_UART_FLAGS_RTS_FLOW            0x0001
#define UMP_MASK_UART_FLAGS_RTS_DISABLE         0x0002
#define UMP_MASK_UART_FLAGS_PARITY              0x0008
#define UMP_MASK_UART_FLAGS_OUT_X_DSR_FLOW      0x0010
#define UMP_MASK_UART_FLAGS_OUT_X_CTS_FLOW      0x0020
#define UMP_MASK_UART_FLAGS_OUT_X               0x0040
#define UMP_MASK_UART_FLAGS_OUT_XA              0x0080
#define UMP_MASK_UART_FLAGS_IN_X                0x0100
#define UMP_MASK_UART_FLAGS_DTR_FLOW            0x0800
#define UMP_MASK_UART_FLAGS_DTR_DISABLE         0x1000 
#define UMP_MASK_UART_FLAGS_RECEIVE_MS_INT      0x2000
#define UMP_MASK_UART_FLAGS_AUTO_START_ON_ERR   0x4000

/*----------------------------------------------------------------------------*/
/* Port Settings Constants  (continued)                                       */
#define UMP_DMA_MODE_CONTINOUS               0x01
#define UMP_PIPE_TRANS_TIMEOUT_ENA           0x80
#define UMP_PIPE_TRANSFER_MODE_MASK          0x03
#define UMP_PIPE_TRANS_TIMEOUT_MASK          0x7C

/*----------------------------------------------------------------------------*/
/* Port Test Settings Constants                                               */
#define UMP_PORT_TESTMODE_INTLOOP            0x00 /* Internal loopback        */
#define UMP_PORT_TESTMODE_EXTLOOP            0x01 /* External loopback        */  


/*----------------------------------------------------------------------------*/
/* Purge port Constants                                                       */
#define UMP_PORT_DIR_OUT                     0x00
#define UMP_PORT_DIR_IN                      0x80  

                                                 
/*----------------------------------------------------------------------------*/
/* UMP firmware commands errors codes                                         */

typedef enum
{
  DFW_ERR_NO_ERROR,
  DFW_ERR_INTERNAL_ERROR,
  DFW_ERR_UNSUFFICIENT_RESOURCES,
  DFW_ERR_INVALID_DATA_LEN_VALUE,
  DFW_ERR_READ_WITHOUT_WRITE,
  DFW_ERR_UNRECOGNIZED_COMMAND,
  DFW_ERR_UNSUPPORTED_COMMAND,
  DFW_ERR_INVALID_MODULE_NUMBER,
  DFW_ERR_INVALID_DATA,
  DFW_ERR_INVALID_TEST_MODE,
  DFW_ERR_INVALID_INTERFACE_MODE,
  DFW_ERR_INVALID_SETTINGS,
  DFW_ERR_INVALID_UART_BAUDRATE,
  DFW_ERR_INVALID_UART_PARITY,
  DFW_ERR_INVALID_UART_STOPBITS,
  DFW_ERR_INVALID_UART_DATABITS,
  DFW_ERR_INVALID_PIPE_MODE_VALUE,
  DFW_ERR_PORT_NOT_STARTED,
  DFW_ERR_PORT_NOT_OPENED,
  DFW_ERR_PORT_IN_USE,
  DFW_ERR_ERROR_DURING_RECEIVE_DATA,
  DFW_ERR_MODE_NOT_SUPPORTED,
  DFW_ERR_EXT_DEVICE_ERROR,
  DFW_ERR_INVALID_I2C_FREQ,
  DFW_ERR_I2C_BUS_ERROR,
  /* Toolkit group                                                            */
  DFW_ERR_DTK_PROTECTION_ERROR = 0x80,
  DFW_ERR_INVALID_TARGET_SPACE,
  DFW_ERR_INVALID_ADDRESS_SIZE,
  DFW_ERR_INVALID_ADDRESS,
  DFW_ERR_INVALID_DATA_SIZE
  } eUMPErrorCode;

// PJG: For god sake man, you don't need to conditional the enums too.
typedef enum
{
  /* UMP modules' identifiers                                                 */
  UMPM_I2C = 0x01,
  UMPM_IEEE1284_PORT,                   
  UMPM_UART1_PORT = 0x03 , 
  UMPM_UART2_PORT  ,    
  UMPM_TUSB_GLCTRL = 0x05   // **** BUGBUG IGX:BK 2002/01/13 - is this true for 3410? *****                    
} eDevFwModule;

typedef enum
{
  /* UMP firmware commands' codes                                             */
  DFW_CMD_GET_VERSION = 1,
  UMPC_GET_PORT_STATUS ,                
  UMPC_GET_PORT_DEV_INFO ,              
  UMPC_GET_CONFIG ,                     
  UMPC_SET_CONFIG ,                     
  UMPC_OPEN_PORT  ,                     
  UMPC_CLOSE_PORT ,                     
  UMPC_START_PORT ,                     
  UMPC_STOP_PORT  ,                     
  UMPC_TEST_PORT  ,                     
  UMPC_PURGE_PORT ,
  UMPC_RESET_EXT_DEVICE ,  
  /* Toolkit commands                                                         */
  /* Read-write group                                                         */
  DFW_CMD_DTK_WRITE = 0x80,
  DFW_CMD_DTK_READ,              
  UMPC_NONEXISTS_CMD = 0xFF
} eDevFwCmdCode;

/*------------------------------------------------------------------------------
  TYPE DEFINITIONS
  Structures for Firmware commands
  ----------------------------------------------------------------------------*/


typedef struct          /* Firmware version and capabilities                  */
{
  BYTE bDfwRevMajor;    /* Device FrameWork revision, major number            */
  BYTE bDfwRevMinor;    /* Device FrameWork revision, minor number            */
  WORD wDfwCapabil;     /* Firmware capabilities                              */
  WORD wMaxDataLength;  /* Maximal supported data length 
                           for data stage for vendor specifica requests       */
  BYTE bAppRevMajor;    /* Application Firmware revision, major number        */
  BYTE bAppRevMinor;    /* Application Firmware revision, minor number        */
} tDevFwVersion, *ptDevFwVersion;

typedef struct          /* Device memory and memory-mapped registers 
                           read / write data header                           */
{                                                                             
  BYTE bAddrType;       /* Address type                                       */
  BYTE bDataType;       /* Data type                                          */
  BYTE bDataCounter;    /* Data counter                                       */
  WORD wBaseAddrHi;     /* Base address (high word)                           */
  WORD wBaseAddrLo;     /* Base address (low word)                            */
} tDevFwDtkRwHdr, *ptDevFwDtkRwHdr;

typedef struct          /* I2C settings                                       */
{
  BYTE bReserved;       /* Reserved for word align                            */
  BYTE bBusSpeed;       /* I2C bus speed:                                     */
                        /* I2C_BUS_SPEED_100 - 100 KHz bus speed,             */
                        /* I2C_BUS_SPEED_400 - 400 KHz bus speed              */
} tDevFwI2cConfig, *ptDevFwI2cConfig;

typedef struct                            /* UMPC_GET_PORT_STATUS results     */
{
  BYTE bPortStatus1;                      /* Port status                      */
  BYTE bPortStatus2;                      /* Port status                      */
} tUmpPortStatus, *ptUmpPortStatus;


typedef struct                            /* UMPC_GET_PORT_DEV_INFO 
                                             results add'l                    */
{
  void *pDevInfo;                         /* Device answer                    */
} tUmpPortDevInfoA, *ptUmpPortDevInfoA;


typedef struct                            /* UMPC_GET_PORT_DEV_INFO 
                                             results                          */
{
  BYTE bDevSupModes;                      /* Supported modes                  */
} tUmpPortDevInfo, *ptUmpPortDevInfo; 


#ifdef UMP_INCLUDE_IEEE1284
typedef struct                            /* IEEE-1284 settings               */
{
  BYTE bPortMode;                         /* IEEE-1284 port mode:             */
  BYTE bAddrReg;                          /* ECP/EPP address register         */
  BYTE bFlags;                            /* Use extended settings            */
} tUmp1284Config, *ptUmp1284Config;
#endif // UMP_INCLUDE_IEEE1284

typedef struct                            /* UART settings                    */
{
  WORD wBaudRate;                         /* Baud rate                        */
  WORD wFlags;                            /* Bitmap mask of flags             */
  BYTE bDataBits;                         /* 5..8 - data bits per character   */
  BYTE bParity;                           /* Parity settings                  */
  BYTE bStopBits;                         /* Stop bits settings               */
  char cXon;                              /* XON character                    */
  char cXoff;                             /* XOFF character                   */
  BYTE bUartMode;                         /* RS-232 or RS-485                 */
} tUmpUartConfig, *ptUmpUartConfig;


typedef struct                            /* End-point settings               */
{
   BYTE bReserved1;                       /* Alignment to word                */
   BYTE bPipeSettings;                    /* Pipe settings                    */
} tUmpEpSettings, *ptUmpEpSettings;



typedef struct                            /* Test port parameters             */
{
  BYTE bReserved1;                        /* Alignment to word                */
  BYTE bTestMode;                         /* Test mode                        */
} tUmpTestPort, *ptUmpTestPort;

#ifdef UMP_INCLUDE_IEEE1284
typedef struct                            /* IEEE-1284 test results           */
{
  BYTE b1284Err;                          /* Errors bitmask                   */
} tUmp1284TestRes, *ptUmp1284TestRes;
#endif // UMP_INCLUDE_IEEE1284

typedef struct                            /* UART test results                */
{
  BYTE  bUartError;                       /*  Xfers errors bitmask            */
} tUmpUartTestRes, *ptUmpUartTestRes;


typedef struct                            /* TUSB settings                    */
{
  BYTE bMask;                             /* Bitmask                          */
  BYTE bTusbCtrlReg;                      /* TUSB5152 global control register */
} tUmpTusbConfig, *ptUmpTusbConfig;



typedef union                             /* Module configuration Data        */
{
  tDevFwI2cConfig     tI2cConfig;
#ifdef UMP_INCLUDE_IEEE1284
  tUmp1284Config      ParPConfig;
#endif // UMP_INCLUDE_IEEE1284
  tUmpUartConfig      UartConfig;
  tUmpTusbConfig      TusbConfig;
} tDevFwModuleConfig, *ptDevFwModuleConfig;


typedef union                             /* UMP_TEST_PORT results            */
{
#ifdef UMP_INCLUDE_IEEE1284
  tUmp1284TestRes     ParPTestRes;
#endif // UMP_INCLUDE_IEEE1284
  tUmpUartTestRes     UartTestRes;
} tUmpTestPortRes, *ptUmpTestPortRes;

typedef struct                            /* Purge port structure             */
{
   BYTE bReserved1;                       /* Alignment to word                */
   BYTE bDirection;                       /* Direction,
                                              either UMP_PORT_DIR_OUT 
                                              or UMP_PORT_DIR_IN              */
} tUmpPurgePortData, *ptUmpPurgePortData; 



typedef union                             /* UMP F/W command specific data    */
{
  tDevFwModuleConfig  tModuleConfig;
  tDevFwDtkRwHdr      tDtkRwHdr;
  tUmpEpSettings      tEpSettings;
  tUmpTestPort        tTestPort;
  tUmpPurgePortData   tPurgePort;
} tDevFwCmdData, *ptDevFwCmdData;

typedef struct
{
  BYTE bCmdCode;        /* Command code                                       */
  BYTE bModuleID;       /* Module ID                                          */
  BYTE bErrorCode;      /* Error code for response                            */
} tDevFwCmdHeader, *ptDevFwCmdHeader;

typedef struct          /* UMP firmware command                               */
{
  tDevFwCmdHeader tCmdHeader;
  tDevFwCmdData   tCmdData;
} tDevFwCmd, *ptDevFwCmd;

typedef union                             /* UMP firmware command results     */
{
  tDevFwVersion       tFwVersion;
  tUmpPortStatus      tPortStatus;
  tDevFwModuleConfig  tModuleConfig;
  tUmpPortDevInfo     tPortDevInfo;
  tUmpTestPortRes     tTestPortRes;
} tDevFwCmdResults, *ptDevFwCmdResults;


typedef struct          /* Command execution response                         */
{
  tDevFwCmdHeader tCmdHeader;
  tDevFwCmdResults tCmdResults;           /* Command execution results        */
} tDevFwCmdResponse, *ptDevFwCmdResponse;


/*------------------------------------------------------------------------------
  TYPE DEFINITIONS
  Structures for USB interrupts
  ----------------------------------------------------------------------------*/

typedef struct                            /* Interrupt packet structure       */
{
  BYTE bICode;                            /* Interrupt code (interrupt num)   */
  BYTE bIInfo;                            /* Interrupt information            */
} tUmpInterrupt, *ptUmpInterrupt;


/*------------------------------------------------------------------------------
  EXTERNAL REFERENCES
  ----------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------
  PUBLIC FUNCTIONS PROTOTYPES
  ----------------------------------------------------------------------------*/


#ifdef   __cplusplus
}
#endif

#ifdef _HOST_
#pragma pack(pop)
#endif


#endif /* _DEV_API_H_                                                         */

/*------------------------------------------------------------------------------
                             END OF FILE
  ----------------------------------------------------------------------------*/
