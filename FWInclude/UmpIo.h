#ifndef _UMPIO_H_
#define _UMPIO_H_
/******************************************************************************/
/*                                                                            */
/*             Copyright (c) 1999 by RealChip Inc.                            */
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
//*   FILE:           UmpIO.h
//*   LANGUAGE:       ANSI C
//*   SUBSYSTEM:      
//*   AUTHOR:         AG
//*   CREATION DATE:  09/20/99
//*
//*   ABSTRACT:       .UMP Device Driver IOCTL API
//*
//* developed for:	Texas Instruments.
//*
//* REVISION HISTORY:
//*
//*  STR #   INIT   DATE            SHORT TITLE
//* [.....]  AG   20 Sep 99 Created.................................................
//* [00174]  DWB  21 NOV 02 Update headers..........................................
//* [xxxxx]  nnn  dd mmm yy ........................................................
//*
//***********************************************************************************
//*

// Include firmware definitions
//
#define _HOST_

#include "..\Include\DevFwApi.h"
#include "..\Include\DevHw.h"

/////////////////////////////////////////////////////////////////////////////
// Device specific definitions

// UMP device endpoints descriptors
//
#define UMP_EP_UART1_IN         0x81
#define UMP_EP_UART1_OUT        0x01

#define UMP_EP_UART2_IN         0x82
#define UMP_EP_UART2_OUT        0x02

#define UMP_EP_1284_IN          0x83
#define UMP_EP_1284_OUT         0x03

#ifdef UMP_PRODUCT_3410
#define UMP_EP_INTERRUPT        0x83
#else  // !UMP_PRODUCT_3410
#define UMP_EP_INTERRUPT        0x87
#endif // !UMP_PRODUCT_3410

// For the w2k version of the driver, it is a run time decision.
#define UMP_EP_INTERRUPT_3410   0x83
#define UMP_EP_INTERRUPT_5x52   0x87

// UMP device UART module configuration flags
//
#define UMP_CFG_UART_BAUDRATE   0x0001
#define UMP_CFG_UART_DATABITS   0x0002
#define UMP_CFG_UART_STOPBITS   0x0004
#define UMP_CFG_UART_PARITY     0x0008
#define UMP_CFG_UART_XON        0x0010
#define UMP_CFG_UART_XOFF       0x0020
#define UMP_CFG_UART_FLAGS      0x4000
#define UMP_CFG_UART_MODE       0x8000
#define UMP_CFG_UART_ALL        UMP_CFG_UART_BAUDRATE   |\
								UMP_CFG_UART_DATABITS   |\
								UMP_CFG_UART_STOPBITS   |\
								UMP_CFG_UART_PARITY     |\
								UMP_CFG_UART_XON        |\
								UMP_CFG_UART_XOFF       |\
								UMP_CFG_UART_FLAGS      |\
								UMP_CFG_UART_MODE       

#ifdef UMP_INCLUDE_IEEE1284
// UMP device 1284 module configuration flags
//
#define UMP_CFG_1284_ADDRESS    0x0001
#define UMP_CFG_1284_FLAGS      0x4000
#define UMP_CFG_1284_MODE       0x8000
#endif // UMP_INCLUDE_IEEE1284

/////////////////////////////////////////////////////////////////////////////
// Registry definition

// UMP port software node value(s)
//
#define UMP_REG_SW_PORT_FLAGS       "UmpPortFlags"
#ifdef UMP_INCLUDE_IEEE1284
#define UMP_REG_SW_LPT_MODE         "UmpLptMode"
#define UMP_REG_SW_LPT_ADDRESS      "UmpLptAddr"
#endif // UMP_INCLUDE_IEEE1284

// UMP port hardware node value(s)
//
#define UMP_REG_HW_PORT             "UmpPort"

/////////////////////////////////////////////////////////////////////////////
// IOCTL constants and data definition

#ifdef WIN32
#pragma pack(push, 1)
#else   // !WIN32
#pragma pack(1)
#endif  // !WIN32

// SUmpIoctlOpen - open request parameters
//
typedef struct
{
    BYTE            m_bModuleID;
    BYTE            m_bMode;
    BYTE            m_bAddr;
} SUmpIoctlOpen, *PSUmpIoctlOpen;

// SUmpIoctlCmd - execute command parameters
//
typedef struct
{
    BYTE            m_bCmdCode;
    BYTE            m_bCmdFlags;
    BYTE            m_bModuleID;
    WORD            m_wValue;
    WORD            m_wTxSize;
    tDevFwCmdData   m_tTxData;
} SUmpIoctlCmd, *PSUmpIoctlCmd;

// Command flags
//
#define UMP_CMDF_READ           (0x01)
#define UMP_CMDF_WRITE          (0x02)
#define UMP_CMDF_READ_WRITE     (UMP_CMDF_READ | UMP_CMDF_WRITE)

// SUmpIoctlResult - request result
//
typedef struct
{
    tDevFwCmdHeader m_hdr;
} SUmpIoctlResult, *PSUmpIoctlResult;

// SUmpIoctlError - request result
//
typedef struct
{
    tDevFwCmdHeader m_hdr;
    DWORD           m_dwError;
} SUmpIoctlGlearError, *PSUmpIoctlGlearError;


#ifdef WIN32
#pragma pack(pop)
#else   // !WIN32
#pragma pack()
#endif  // !WIN32

/////////////////////////////////////////////////////////////////////////////
// DWB IO control codes
//
#define UMP_MAKE_IOCTL(method,code)\
    (ULONG)CTL_CODE(0x8000, 0x800+(code), method, FILE_ANY_ACCESS)

// IOCTL_UMP_PORT_OPEN - open UMP port
//
#define IOCTL_UMP_PORT_OPEN  \
    UMP_MAKE_IOCTL(METHOD_BUFFERED, 0)

// IOCTL_UMP_PORT_CLOSE - close current UMP port
//
#define IOCTL_UMP_PORT_CLOSE \
    UMP_MAKE_IOCTL(METHOD_BUFFERED, 1)

// IOCTL_UMP_PORT_CLEAR_ERROR - clear UMP port error
//
#define IOCTL_UMP_PORT_CLEAR_ERROR \
    UMP_MAKE_IOCTL(METHOD_BUFFERED, 2)

// IOCTL_UMP_COMMAND - execute UMP command
//
#define IOCTL_UMP_COMMAND   \
    UMP_MAKE_IOCTL(METHOD_BUFFERED, 3)

////////////////////////////////////////////////////////////////////////////////

#endif // _UMPIO_H_

