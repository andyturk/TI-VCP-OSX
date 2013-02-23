#ifndef _DEV_HW_H_
#define _DEV_HW_H_
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
//*   FILE:           DevHw.h
//*   LANGUAGE:       ANSI C
//*   SUBSYSTEM:      
//*   AUTHOR:         AG
//*   CREATION DATE:  09/20/99
//*
//*   ABSTRACT:       TI TUSB5152 (UMP) project.
//*				            UMP Register address & constant variable definition file.
//*
//* developed for:	Texas Instruments.
//*
//* REVISION HISTORY:
//*
//*   Date         By       Description
//*   -----------  -----    --------------------------------------------
//*   Apr-07-1999  Murali   Created
//*   May-13-1999  IVK      Add TUSB5152 Vector Interrupt Register definitions
//*   May-17-1999  IVK      Add a lot of stuff
//*   Jan-08-2000  NMD      Modified to suit to DFW.
//*   Jan-21-2000  NMD      UMP code release 0.51 done with new DFW.
//*   Feb-04-2000  NMD      UMP code release 0.52 done with new DFW.
//*
//*
//*  STR #   INIT   DATE            SHORT TITLE
//* [00174]  DWB  21 NOV 02 Update headers..........................................
//* [xxxxx]  nnn  dd mmm yy ........................................................
//*
//***********************************************************************************
//*

#ifdef  __cplusplus
extern  "C" 
{
#endif


#define UMP_EPS_QTY                       7
#define UMP_UARTS_QTY                     2
#define UMPM_QTY1                         6      /* number of modules that 
                                                    require store current     */

#define INT_EX0_ISR_ADDR               0x03

/* Interrupts vector related declaration and definitions                      */
#define INT_VECTOR_REGISTER_ADDR       0xFF92  /* Interrupt vector register   */
#define rIntVector (*((BYTE xdata *)INT_VECTOR_REGISTER_ADDR)) 

/* UMP global control register's definitions                                  */
#define GLBL_CONTROL_ADDR              0xFF91  /* Addr of global control reg  */
#define rGlblControl (*((BYTE xdata *)GLBL_CONTROL_ADDR))

#define ROM_SHADOW_REGITER_ADDR        0xFF90
#define rRomSdw (*((BYTE xdata *)ROM_SHADOW_REGITER_ADDR))


#define DEV_MPU_HI_FREQ_FLAG             0x80

#define USB_CMD_DATA_MAXLEN              0x7F

/* USB registers' constants                                                   */

#define USB_BUFFER_ADDRESS_SHIFT         0x03  /* Address shift for store to 
                                                  EP descriptor blocks        */
#define USB_BUFFER_ADDRESS_BASE        0xF800                                                   
#define DFW_EP0_COUNTER_MASK             0x0F
#define DFW_EPN_COUNTER_MASK             0x7F

#define USB_EPCFG_USBIE_FLAG             0x04  /* MCU interrupt enable        */
#define USB_EPCFG_STALL_FLAG             0x08  
#define USB_EPCFG_DBUF_FLAG              0x10  /* Double buffers are used     */
#define USB_EPCFG_TOGLE_FLAG             0x20  
#define USB_EPCFG_ISO_FLAG               0x40  
#define USB_EPCFG_UBME_FLAG              0x80  /* UBM enable for EP           */

#define USB_EPCNT_NAK_FLAG               0x80

#define USB_STATUS_STPOW_FLAG            0x01  /* Setup overwrite flag        */
#define USB_STATUS_SETUP_FLAG            0x02  /* Setup flag                  */
#define USB_STATUS_RESR_FLAG             0x20  /* Resume flag                 */
#define USB_STATUS_SUSR_FLAG             0x40  /* Suspend flag                */
#define USB_STATUS_RSTR_FLAG             0x80  /* Reset flag                  */

#define USB_INTMASK_STPOW_FLAG           0x01  /* Setup overwrite int mask    */
#define USB_INTMASK_SETUP_FLAG           0x04  /* Setup int mask              */
#define USB_INTMASK_RESR_FLAG            0x20  /* Resume int mask             */
#define USB_INTMASK_SUSR_FLAG            0x40  /* Suspend int mask            */
#define USB_INTMASK_RSTR_FLAG            0x80  /* Reset int mask              */

#define USB_CONTROL_INDIR_FLAG           0x01  /* Direction                   */
#define USB_CONTROL_SIPRG_FLAG           0x02  /* Setup in progress           */
#define USB_CONTROL_FRSTE_FLAG           0x10  /* Function reset connection   */
#define USB_CONTROL_RWUP_FLAG            0x20  /* Remote wake-up request      */
#define USB_CONTROL_FEN_FLAG             0x40  /* Function enable             */
#define USB_CONTROL_CONT_FLAG            0x80  /* Connect/Disconnect          */ 


#define USB_CONTROL_ADDR               0xFFFC  /* Addr of USB control reg     */
#define USB_INTMASK_ADDR               0xFFFD  /* Addr of USB ints mask reg   */
#define USB_STATUS_ADDR                0xFFFE  /* Addr of USB status register */                     
#define USB_FUNADDR_ADDR               0xFFFF  /* Addr of function address 
                                                  register                    */

#define rUsbControl (*((BYTE xdata *)USB_CONTROL_ADDR))
#define rUsbIntMask (*((BYTE xdata *)USB_INTMASK_ADDR))
#define rUsbStatus  (*((BYTE xdata *)USB_STATUS_ADDR))
#define rUsbFunAddr (*((BYTE xdata *)USB_FUNADDR_ADDR))

/* USB address space constants                                                */
#define XYBUFS_TOP_SPACE_ADDR          0xFEF0  /* We allocate buffers from top
                                                  and expand it downward,
                                                  because free space is used 
                                                  for firmware variables. These 
                                                  vars occupy space from start,
                                                  expanding upward.           */     

#define UMPU_EPBCT_NAK       0x80        /* NAK                               */

#define UMPD_IEDB_ADDRESS    0xFF48      /* Input EDBs Base Address           */
#define UMPD_OEDB_ADDRESS    0xFF08      /* Output EDBs Base Address          */


#define UMPR_HUBVIDH         0xFFFB      /* HUB VID High-byte Register        */
#define UMPR_HUBVIDL         0xFFFA      /* HUB VID Low-byte Register         */
#define UMPR_HUBPIDH         0xFFF9      /* HUB PID High-byte Register        */
#define UMPR_HUBPIDL         0xFFF8      /* HUB PID Low-byte Register         */

/*                          DMA registers                                     */
#define UMPR_DMACDR1         0xFFE0      /* DMA Channel 1 Definition Register */
#define UMPR_DMACDR2         0xFFE2      /* DMA Channel 2 Definition Register */
#define UMPR_DMACDR5         0xFFE8      /* DMA Channel 5 Definition Register */
#define UMPR_DMACSR5         0xFFE9      /* DMA Channel 5 Control & Status    */

#define UMPR_DMA_XY_BIT_MASK   0x10   
#define UMPR_DMA_XY1_BIT_MASK  0x08   
#define UMPR_DMA_CNT_MASK      0x20      /* Continous mode mask               */
#define UMPR_DMA_INE_MASK      0x40      /* DMA interrupts enable             */
#define UMPR_DMA_ENA_MASK      0x80      /* DMA enabled                       */

#define UMPR_DMA_YBUF_MASK     0x08
#define UMPR_DMA_EP_MASK       0x07
#define UMPR_DMA_ADDR_SHIFT    0x04

#define UMPR_DMA_ADDR_BASE   0xF800 

#define UMP_DMA_TIMEOUT_MIN    0x04 

#define UMP_DMA_QTY            0x05


/*                          UART Registers                                    */

#define UMPMEM_BASE_UART1          0xFFA0  /* UMP UART1 base address          */
#define UMPMEM_BASE_UART2          0xFFB0  /* UMP UART2 base address          */
                                                                              
#define UMPMEM_OFFS_UART_LCR       0x02    /* UMP UART LCR register offset    */
#define UMPMEM_OFFS_UART_FCR       0x03    /* UMP UART FCR register offset    */
#define UMPMEM_OFFS_UART_MCR       0x04    /* UMP UART MCR register offset    */
#define UMPMEM_OFFS_UART_LSR       0x05    /* UMP UART LSR register offset    */
#define UMPMEM_OFFS_UART_MSR       0x06    /* UMP UART MSR register offset    */



#define UMPR_RDR1            UMPMEM_BASE_UART1
#define UMPR_LSR1            ( UMPMEM_BASE_UART1 + UMPMEM_OFFS_UART_LSR ) 
#define UMPR_MSR1            ( UMPMEM_BASE_UART1 + UMPMEM_OFFS_UART_MSR ) 
#ifdef UMP_INCLUDE_UART2
#define UMPR_LSR2            ( UMPMEM_BASE_UART2 + UMPMEM_OFFS_UART_LSR ) 
#define UMPR_MSR2            ( UMPMEM_BASE_UART2 + UMPMEM_OFFS_UART_MSR ) 
#endif // UMP_INCLUDE_UART2


/* Flow control register masks and constants                                  */
#define UMP_UART_FCR_OUT_XF        0x01
#define UMP_UART_FCR_OUT_XA        0x02
#define UMP_UART_FCR_CTS_FLOW      0x04
#define UMP_UART_FCR_DSR_FLOW      0x08
#define UMP_UART_FCR_IN_X          0x10
#define UMP_UART_FCR_RTS_FLOW      0x20 
#define UMP_UART_FCR_DTR_FLOW      0x40
#define UMP_UART_FCR_MODE_MASK     0x80       
#define UMP_MASK_FCR_FLOW_ENA      0x7F 
#define UMP_UART_MODE_SHIFT           7 /* Shift for FCRL register to obtain 
                                           port mode constant UMP_UART_xxxMODE
                                           (see UmpApi.h)                     */ 
#define UMP_UART_FCR_OUT_X     (UMP_UART_FCR_OUT_XF | UMP_UART_FCR_OUT_XA)

/* Modem control register masks                                               */
#define UMP_UART_MCR_RST_MASK      0x01
#define UMP_UART_MCR_RE_MASK       0x02
#define UMP_UART_MCR_DTR_ENABLE    0x10
#define UMP_UART_MCR_RTS_ENABLE    0x20

/* Line control register masks                                                */
#define UMP_UART_LCR_PARITY_MASK   0x38
#define UMP_UART_LCR_FIFO_MASK     0x80
#define UMP_UART_LCR_NO_PARITY     0x00
#define UMP_UART_LCR_ODD_PARITY    0x08
#define UMP_UART_LCR_EVN_PARITY    0x18
#define UMP_UART_LCR_SPC_PARITY    0x28
#define UMP_UART_LCR_MRK_PARITY    0x38
#define UMP_UART_LCR_STOPBITS_MASK 0x04
#define UMP_UART_LCR_DATALEN_MASK  0x03

#define UMP_UART_STOP_SHIFT           1       
#define UMP_UART_PARITY_DIV        0x10    
#define UMP_UART_WL_MASK           0x03  /* Byte size                         */                                      
#define UMP_UART_STP_MASK          0x04  /* Stop bits                         */                                      
#define UMP_UART_PRTY_MASK         0x08  /* Parity enable/disable             */
#define UMP_UART_EPRTY_MASK        0x10  /* Even - odd bit                    */                                      
#define UMP_UART_FPTY_MASK         0x20  /* Forced parity mask                */                                      
#define UMP_UART_BRK_MASK          0x40  /* Break control bit mask            */                                      
#define UMP_UART_FEN_MASK          0x80  /* FIFO disable                      */
#define UMP_UART_WL_BASE           0x05  /* Value to convert byte from UMP 
                                            register to host settings         */
                                            

#define UMP_MASK_UART_FLAGS_HWSET   0x19F3
#define UMP_MASK_UART_FLAGS_IGNORED 0xC604    


/* Modem status register masks                                                */
#define UMP_UART_MSR_LDSR_MASK     0x20
#define UMP_UART_MSR_INF_MASK      0x0F
#define UMP_UART_MCR_LOOP_MASK     0x04

/* Line status register masks                                                 */
#define UMP_UART_LSR_OV_MASK       0x01
#define UMP_UART_LSR_PE_MASK       0x02
#define UMP_UART_LSR_FE_MASK       0x04
#define UMP_UART_LSR_BR_MASK       0x08
#define UMP_UART_LSR_ER_MASK       0x0F
#define UMP_UART_LSR_RX_MASK       0x10
#define UMP_UART_LSR_TX_MASK       0x20

/* UART interrupts register masks                                             */
#define UMP_UART_IMR_MSIE          0x01
#define UMP_UART_IMR_LSIE          0x02
#define UMP_UART_IMR_TRIE          0x04

#ifdef UMP_INCLUDE_IEEE1284
/*                          IEEE 1284 Registers                               */
#define    UMPR_PPMCR            0xFF9A  /* P-Port Mode Control Register      */
#define    UMPR_PPIMSK           0xFF9B  /* P-Port Interrupt Mask Register    */
#define    UMPR_PPSTA            0xFF9C  /* P-Port Status Register            */
#define    UMPR_PPCTL            0xFF9D  /* P-Port Control Register           */
#define    UMPR_PPDAT            0xFF9E  /* P-Port Data Register              */
#define    UMPR_PPADR            0xFF9F  /* P-Port EPP ECP Address Register   */
#define    UMP_1284_NBL_MODE       0x05  
#define    UMP_1284_IN_DIR         0x08

#define UMP_IEEE1284_CENTRONICS    0x00
#define UMP_IEEE1284_NBL           0x05 
#define UMP_IEEE1284_DEVID         0x04
#endif // UMP_INCLUDE_IEEE1284

/* I2C registers' constants                                                   */
#define I2C_STS_CTL_ADDR               0xFFF0  /* Addr of I2C control reg     */
#define I2C_ADDR_ADDR                  0xFFF3  /* Addr of I2C control reg     */
#define I2C_OUT_ADDR                   0xFFF1  /* Addr of I2C output reg      */
#define I2C_IN_ADDR                    0xFFF2  /* Addr of I2C input reg       */                     

#define rI2cStsCtl   (*((BYTE xdata *)I2C_STS_CTL_ADDR))
#define rI2cAddr     (*((BYTE xdata *)I2C_ADDR_ADDR))
#define rI2cOut      (*((BYTE xdata *)I2C_OUT_ADDR))
#define rI2cIn       (*((BYTE xdata *)I2C_IN_ADDR))

#define I2C_STOP_WRITE                   0x01
#define I2C_STOP_READ                    0x02
#define I2C_TRANSMIT_INT_MASK            0x04
#define I2C_TX_EMPTY                     0x08
#define I2C_BUS_SPEED_400_MASK           0x10
#define I2C_BUS_ERROR_MASK               0x20
#define I2C_RECEIVE_INT_MASK             0x40
#define I2C_RX_FULL                      0x80

/*                          Vector Interrupt values                           */
#define    NO_INTERRUPT         0x00
#define    OEP1_INTERRUPT       0x12  /* Output-Endpoint-1 interrupt     */
#define    OEP2_INTERRUPT       0x14  /* Output-Endpoint-2 interrupt     */
#define    OEP3_INTERRUPT       0x16  /* Output-Endpoint-3 interrupt     */
#define    OEP4_INTERRUPT       0x18  /* Output-Endpoint-4 interrupt     */
#define    OEP5_INTERRUPT       0x1A  /* Output-Endpoint-5 interrupt     */
#define    OEP6_INTERRUPT       0x1C  /* Output-Endpoint-6 interrupt     */
#define    OEP7_INTERRUPT       0x1E  /* Output-Endpoint-7 interrupt     */

#define    IEP1_INTERRUPT       0x22  /* Input-Endpoint-1 interrupt      */
#define    IEP2_INTERRUPT       0x24  /* Input-Endpoint-2 interrupt      */
#define    IEP3_INTERRUPT       0x26  /* Input-Endpoint-3 interrupt      */
#define    IEP4_INTERRUPT       0x28  /* Input-Endpoint-4 interrupt      */
#define    IEP5_INTERRUPT       0x2A  /* Input-Endpoint-5 interrupt      */
#define    IEP6_INTERRUPT       0x2C  /* Input-Endpoint-6 interrupt      */
#define    IEP7_INTERRUPT       0x2E  /* Input-Endpoint-7 interrupt      */

#define    STPOW_INTERRUPT      0x30  /* STPOW packet received           */
#define    SETUP_INTERRUPT      0x32  /* SETUP packet received           */
#define    RESR_INTERRUPT       0x38  /* RESR interrupt                  */
#define    SUSR_INTERRUPT       0x3A  /* SUSR interrupt                  */
#define    RSTR_INTERRUPT       0x3C  /* RSTR interrupt                  */

#define    IEP0_INTERRUPT       0x44  /* Input-Endpoint-0 interrupt      */
#define    OEP0_INTERRUPT       0x46  /* Output-Endpoint-0 interrupt     */


#define    UART1_STS_INTERRUPT  0x50  /* UART1 status register interrupt */
#define    UART1_MDM_INTERRUPT  0x52  /* UART1 modem register interrupt  */
#ifdef UMP_INCLUDE_UART2
#define    UART2_STS_INTERRUPT  0x54  /* UART2 status register interrupt */
#define    UART2_MDM_INTERRUPT  0x56  /* UART2 modem register interrupt  */
#endif // UMP_INCLUDE_UART2

#define    UART1_RX_INTERRUPT   0x60  /* UART1 RxF interrupt             */
#define    UART1_TX_INTERRUPT   0x62  /* UART1 TxE interrupt             */
#ifdef UMP_INCLUDE_UART2
#define    UART2_RX_INTERRUPT   0x64  /* UART2 RxF interrupt             */
#define    UART2_TX_INTERRUPT   0x66  /* UART2 TxE interrupt             */
#endif // UMP_INCLUDE_UART2

#ifdef UMP_INCLUDE_IEEE1284
#define    PP_RX_INTERRUPT      0x70  /* IEEE1284 port RxF interrupt     */
#define    PP_TX_INTERRUPT      0x72  /* IEEE1284 port TxE interrupt     */
#define    PP_FALT_INTERRUPT    0x74  /* IEEE1284 port FALT interrupt    */
#define    PP_ACK_INTERRUPT     0x76  /* IEEE1284 port ACK interrupt     */
#define    PP_PE_INTERRUPT      0x78  /* IEEE1284 port PE  interrupt     */

#define    PP_INTERRUPT_MASK_RX 0x01  /* IEEE1284 port RX interrupt ena  */
#define    PP_INTERRUPT_MASK_TX 0x02  /* IEEE1284 port TX interrupt ena  */
#define    PP_INTERRUPT_MASK_FL 0x08  /* IEEE1284 port FAULT interrupt   */
#define    PP_INTERRUPT_MASK_PE 0x20  /* IEEE1284 port PE interrupt ena  */
#define    PP_INTERRUPT_MASK_AK 0x40  /* IEEE1284 port ACK interrupt ena */
#endif // UMP_INCLUDE_IEEE1284

#define    DMA1_INTERRUPT       0x80  /* DMACDR1 interrupt               */
#define    DMA2_INTERRUPT       0x82  /* DMACDR2 interrupt               */
#define    DMA3_INTERRUPT       0x84  /* DMACDR3 interrupt               */
#define    DMA4_INTERRUPT       0x86  /* DMACDR4 interrupt               */
#define    DMA5_INTERRUPT       0x88  /* DMACDR5 interrupt               */


#define    UART_INTERRUPT_MASK  0x08  /* UART port interrupts enable     */

typedef struct                /* Endpoint N descriptor block                  */
{
  BYTE    bEpConfig;          /* Endpoint Configuration                       */
  BYTE    bEpBBAX;            /* Endpoint X Buffer Base Address               */
  BYTE    bEpBCtX;            /* Endpoint X Buffer byte Count                 */
  BYTE    bSpare0;            /* no used                                      */
  BYTE    bSpare1;            /* no used                                      */
  BYTE    bEpBBAY;            /* Endpoint Y Buffer Base Address               */
  BYTE    bEpBCtY;            /* Endpoint Y Buffer byte Count                 */
  BYTE    bEpSizeXY;          /* Endpoint XY Buffer Size                      */
} tEpNDb, *ptEpNDb;

typedef struct                /* Endpoint 0 descriptor block                  */
{
  BYTE    bIEpConfig;         /* Input Endpoint 0 Configuration Register      */
  BYTE    bIEpBCnt;           /* Input Endpoint 0 Buffer Byte Count           */
  BYTE    bOEpConfig;         /* Output Endpoint 0 Configuration Register     */
  BYTE    bOEpBCnt;           /* Output Endpoint 0 Buffer Byte Count          */
} tEp0Db, *ptEp0Db;

typedef struct
{
  BYTE      bRdr;               /* Receiver data register                     */
  BYTE      bTdr;               /* Transmitter data register                  */
  BYTE      bLcr;               /* Line control register                      */
  BYTE      bFcr;               /* Flow control register                      */
  BYTE      bMcr;               /* Modem control register                     */
  BYTE      bLsr;               /* Line status register                       */
  BYTE      bMsr;               /* Modem status register                      */
  BYTE      bDlr;               /* Divisor low byte register                  */
  BYTE      bDhr;               /* Divisor high byte register                 */
  BYTE      bXon;               /* Xon register                               */
  BYTE      bXoff;              /* Xoff register                              */
  BYTE      bImr;               /* Interrupt mask register                    */
  BYTE      bRsrv2;             /* Reserved for alignment                     */
  BYTE      bRsrv3;             /* Reserved for alignment                     */
  BYTE      bRsrv4;             /* Reserved for alignment                     */
  BYTE      bRsrv5;             /* Reserved for alignment                     */
  
} tUmpUartRegs, *ptUmpUartRegs;  

typedef struct
{
  BYTE      bCdr1;              /* Channel definition register                */
  BYTE      bCsr1;              /* Control & status register                  */
  BYTE      bRsrv1;             /* Reserved for alignment                     */
  BYTE      bRsrv2;             /* Reserved for alignment                     */
  BYTE      bCdr2;              /* Channel definition register                */
  BYTE      bCsr2;              /* Control & status register                  */
} tUmpUartDmaRegs, *ptUmpUartDmaRegs;

typedef struct
{
  BYTE      bCdr;               /* Channel definition register                */
  BYTE      bCsr;               /* Control & status register                  */
} tUmpDmaRegs, *ptUmpDmaRegs;

#ifdef   __cplusplus
}
#endif

#endif // _DEV_HW_H_


