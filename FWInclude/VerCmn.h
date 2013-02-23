#ifndef _VERCMN_H_
#define _VERCMN_H_
/******************************************************************************/
/*                                                                            */
/*             Copyright (c) 1998,1999 by RealChip Inc.                       */
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
//*   FILE:           VerCmn.h
//*   LANGUAGE:       ANSI C
//*   SUBSYSTEM:      
//*   AUTHOR:         Paul J. Gryting  (IGX)
//*   CREATION DATE:  xx
//*
//*   ABSTRACT:       Common version resource definitions
//*
//* developed for:	Texas Instruments.
//*
//* REVISION HISTORY:
//*
//*  STR #   INIT   DATE            SHORT TITLE
//* [.....]  AG   19 Jun 99 Created.................................................
//* [00174]  DWB  21 NOV 02 Update headers..........................................
//* [xxxxx]  nnn  dd mmm yy ........................................................
//*
//***********************************************************************************
//*

/* ******************************** */
/*
 * The build procedure must define exactly one of:
 *	UMP_PRODUCT_3410
 *	UMP_PRODUCT_5052
 *	UMP_PRODUCT_5152
 *
 * This switch determines whether the code and comments in UMPUSB.SYS include
 * support for the IEEE1284 parallel port on the TUSB5152 chip (which is not
 * present on the TUSB5052 or TUSB3410 chip).  The version resource for the
 * file is also set accordingly, although UMPMON.DLL does not have any other
 * differences.
 *
 */

#if defined(UMP_PRODUCT_3410) && defined(UMP_PRODUCT_5052)
#error ======== MULTIPLE UMP_PRODUCT_XXXX SYMBOLS DEFINED ========
#elif defined(UMP_PRODUCT_3410) && defined(UMP_PRODUCT_5152)
#error ======== MULTIPLE UMP_PRODUCT_XXXX SYMBOLS DEFINED ========
#elif defined(UMP_PRODUCT_5052) && defined(UMP_PRODUCT_5152)
#error ======== MULTIPLE UMP_PRODUCT_XXXX SYMBOLS DEFINED ========
#endif


#define RESET_BOTH_ENDPOINTS
// #define MGR_PERIODIC_CHECK_RXDEAD
/* ******************************** */
/*
#ifdef RESET_BOTH_ENDPOINTS
#define VER_PRODUCTVERSION_STR      "1.0.022RBE\0"
#else  // !RESET_BOTH_ENDPOINTS
#define VER_PRODUCTVERSION_STR      "1.0.022\0"
#endif // !RESET_BOTH_ENDPOINTS
*/

#define UMP_INCLUDE_UART2        //Needed for all except 3410
#define UMP_INCLUDE_IEEE1284		 //Force everything to be built for XP driver.

#ifdef UMP_XP_DRIVER
#define VER_OSTYPE_STR             "®Windows 2K/XP"
#define VER_LEGALCOPYRIGHT_STR     "Copyright © Texas Instruments, 1999-2003\0"
#define VER_PRODUCTVERSION_STR     "1.2.10.6 EB\0"  //Major.Minor.Build.Revision
#define VER_PRODUCTVERSION          1,2,10,6

// PJG: multiple chipset product name string for w2k/xp driver.
#define PRODUCTNAME_STR            "(TUSB5052, TUSB3410)"

#else  //UMP_XP_DRIVER

#define VER_OSTYPE_STR             "®Windows 98/ME"
#define VER_LEGALCOPYRIGHT_STR      "Copyright © RealChip, 1999-2000\0"
#define VER_PRODUCTVERSION_STR      "1.2.0.0\0"  //Major.Minor.Build.Revision
#define VER_PRODUCTVERSION          1,2,0,0

#ifdef UMP_PRODUCT_5152
#define PRODUCTNAME_STR         "(TUSB5152)"
#else // UMP_PRODUCT_5152

#undef UMP_INCLUDE_IEEE1284		 //Not needed for 3410/5052

#ifdef UMP_PRODUCT_5052
#define PRODUCTNAME_STR         "(TUSB5052)"
#else // UMP_PRODUCT_5052

#ifdef UMP_PRODUCT_3410
#undef UMP_INCLUDE_UART2		 //Not needed for 3410
#define PRODUCTNAME_STR         "(TUSB3410)"
#else // UMP_PRODUCT_3410

#ifndef UMP_PRODUCT_GENERIC
#error ======== NO UMP_PRODUCT_XXXX SYMBOL DEFINED ========
#endif // UMP_PRODUCT_GENERIC

#endif // UMP_PRODUCT_3410

#endif // UMP_PRODUCT_5052

#endif // UMP_PRODUCT_5152

#endif  //UMP_XP_DRIVER

#ifdef UMP_PRODUCT_GENERIC
#define PRODUCTNAME_STR         "(GENERIC)"
#endif

#define VER_PRODUCTNAME_STR         "Texas Instruments UMP Device Driver " PRODUCTNAME_STR "\0"
#define VER_COMPANYNAME_STR         "Texas Instruments\0"
#define VER_LEGALTRADEMARKS_STR     "\0"
#define VER_BUILDDATE_STR			__DATE__ /* used in debug build only */
#define VER_BUILDTIME_STR			__TIME__ /* used in debug build only */

#endif /* _VERCMN_H_ */

