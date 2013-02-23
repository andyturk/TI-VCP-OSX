#include "TIUMPPrimitives.h"
#include <IOKit/IOMemoryDescriptor.h>

UInt8 frmware3410[] = {
    #include "FWInclude/firmware3410.inc"
};

UInt8 frmware5052[] = {
    #include "FWInclude/firmware5052.inc"
};

#define INTERRUPT_BUFF_SIZE sizeof(tUmpInterrupt)
#define USBLapPayLoad       2048
#define kDefaultBaudRate    9600
#define kMaxBaudRate        460800     // 4Mbs for IrDA
#define kMinBaudRate        300
#define kMaxCirBufferSize   4096

#define propertyTag		"Product Name"


static UInt8 Asciify(UInt8 i)
{

    i &= 0xF;
    if ( i < 10 )
	 return( '0' + i );
    else return( 55  + i );
    
}/* end Asciify */


#define dumplen     32      // Set this to the number of bytes to dump and the rest should work out correct

#define buflen      ((dumplen*2)+dumplen)+3
#define Asciistart  (dumplen*2)+3

/****************************************************************************************************/
//
//      Function:   DEVLogData
//
//      Inputs:     Dir - direction, Count - number of bytes, buf - the data
//
//      Outputs:    None
//
//      Desc:       Puts the data in the log. 
//
/****************************************************************************************************/

IORecursiveLock* gDebugLock = IORecursiveLockAlloc();

char LocBuf[buflen+1];

void DEVLogData(UInt8 Dir, UInt32 Count, char *buf)
{
    UInt8       wlen, i, Aspnt, Hxpnt;
    UInt8       wchr;
    //char        LocBuf[buflen+1];
	
#ifndef SHOW_DEBUG_STRINGS
	return;
#endif

	IORecursiveLockLock(gDebugLock);
	
	for ( i=0; i<buflen; i++ )
    {
		LocBuf[i] = 0x20;
    }
	
    LocBuf[buflen] = 0x00;
    
    if ( Dir == kUSBIn )
    {
	DLOG( "USBLogData - Received, size = %8x\n", Count );
    } else {
	if ( Dir == kUSBOut )
	{
	    DLOG( "USBLogData - Write, size = %8x\n", Count );
	} else {
	    if ( Dir == kUSBAnyDirn )
	    {
		DLOG( "USBLogData - Other, size = %8x\n", Count );
	    }
	}           
    }

    if ( Count > dumplen )
    {
	wlen = dumplen;
    } else {
	wlen = Count;
    }
    
    if ( wlen > 0 )
    {
	Aspnt = Asciistart;
	Hxpnt = 0;
	for ( i=0; i<wlen; i++ )
	{
	    wchr = buf[i];
	    LocBuf[Hxpnt++] = Asciify( wchr >> 4 );
	    LocBuf[Hxpnt++] = Asciify( wchr );
	    if (( wchr < 0x20) || (wchr > 0x7F ) || wchr == '%')       // Non printable characters
	    {
			LocBuf[Aspnt++] = 0x2E;                 // Replace with a period
	    } else {
			LocBuf[Aspnt++] = wchr;
	    }
	}
	
	LocBuf[Aspnt] = 0x00;
	
	DLOG( "%s", LocBuf );
	DLOG( "\n" );
    } else {
	DLOG( "USBLogData - No data, Count = 0\n" );
    }
	
	IORecursiveLockUnlock(gDebugLock);
}/* end DEVLogData */


TIUMPDevice::TIUMPDevice(TIUMPSerial *inService, IOUSBDevice *inUSBDevice, ChipIDT inChipID)
{
	DLOG("TIUMPDevice_c\n");

	mUSBDevice = inUSBDevice;
    mChipID = inChipID;
    mService = inService;
    mPortNum = 0;
    mPortList = NULL;
    mListAllocSize = 0;
    mIntPipe = NULL;
	mTerminate = false;
    
    mIntPipeMDP = IOBufferMemoryDescriptor::withCapacity( INTERRUPT_BUFF_SIZE, kIODirectionIn );
    mIntPipeMDP->setLength( INTERRUPT_BUFF_SIZE );
    
    mIntPipeBuffer = (UInt8*)mIntPipeMDP->getBytesNoCopy();
    
    mIntCompletionInfo.target = this;
    mIntCompletionInfo.action = interruptReadComplete;
    mIntCompletionInfo.parameter  = NULL;
    
    mIntPipeStarted = false;

    mControlPipeInMDP = IOBufferMemoryDescriptor::withCapacity( 255, kIODirectionIn );
    mControlPipeInMDP->setLength( 255 );
    
    mControlPipeInBuffer = (UInt8*)mControlPipeInMDP->getBytesNoCopy();

    mControlPipeOutMDP = IOBufferMemoryDescriptor::withCapacity( 255, kIODirectionOut );
    mControlPipeOutMDP->setLength( 255 );
    
    mControlPipeOutBuffer = (UInt8*)mControlPipeOutMDP->getBytesNoCopy();
}

TIUMPDevice::~TIUMPDevice()
{
    DLOG("~TIUMPDevice\n");
	
	StopIntPolling();
    
    if (mPortList) {
        for (UInt32 i=0;i<mPortNum;i++) {
			TIUMPPort *port=mPortList[i];
			if (port != NULL) {
				IORS232SerialStreamSync *rs232Stream = port->GetStream();
				
				if (rs232Stream)
					rs232Stream->detach(mService);
					
				delete mPortList[i];
			}
        }
        
        IOFree(mPortList, mListAllocSize);
		
		mPortList = NULL;
    }
    
    if (mIntPipe) {
        mIntPipe->release();
    }
	
	if (mIntPipeMDP) {
		mIntPipeMDP->release();
		mIntPipeMDP = 0;
	}
	
	if (mControlPipeInMDP) {
		mControlPipeInMDP->release();
		mControlPipeInMDP = 0;
	}

	if (mControlPipeOutMDP) {
		mControlPipeOutMDP->release();
		mControlPipeOutMDP = 0;
	}
}

void TIUMPDevice::Terminate()
{
    if (mPortList) {
        for (UInt32 i=0;i<mPortNum;i++)
		{
			TIUMPPort *port=mPortList[i];

			port->Release();
		}
	}
}


void TIUMPDevice::interruptReadComplete( void *obj, void *param, IOReturn rc, UInt32 remaining )
{
    TIUMPDevice  *me = (TIUMPDevice*)obj;
    //PortInfo_t            *port = (PortInfo_t*)param;
    IOReturn    ior;
    
    DLOG("interruptReadComplete - error %08X\n",rc);
    
    if ( rc == kIOReturnSuccess )   /* If operation returned ok:    */
    {
		if (remaining == 0) {		
            ptUmpInterrupt inter = (ptUmpInterrupt)me->mIntPipeBuffer;
						
			//only MODEM_STATUS interests us!
			if ((inter->bICode & 0x0F) == 0x4 || (inter->bICode & 0x0F) == 0x3) //UMPI_PUART_DATA_ERROR || UMPI_PUART_MODEM_STATUS
			{				
				 DLOG("interruptReadComplete - event %02X - data %02X\n",inter->bICode, inter->bIInfo);
				
				int portid = (inter->bICode>>4);
				
				for (UInt32 i=0;i<me->mPortNum;i++) {
					TIUMPPort *port = me->mPortList[i];
									
					if (port && port->GetPortID() == portid) {
						tUmpPortStatus hwState;
						
						port->GetHWStatus(hwState);
						
						if ((inter->bICode & 0x0F) == 0x4) //UMPI_PUART_MODEM_STATUS
						{
							hwState.bPortStatus1 = inter->bIInfo;
						}
						else
						{
							hwState.bPortStatus2 = inter->bIInfo;
						}
						
						DLOG("calling  SetHWStatus\n");
						port->SetHWStatus(hwState);
					}
				}
			}
        }
        
        /* Queue the next interrupt read:   */        
		ior = me->mIntPipe->Read( me->mIntPipeMDP, &me->mIntCompletionInfo, NULL );
		if ( ior == kIOReturnSuccess ) {
			return;
		}
    }

}/* end interruptReadComplete */


int TIUMPDevice::StartIntPolling()
{
    if (!mIntPipeStarted && mIntPipe) {
        mIntPipeStarted = true;
        mIntPipe->Read( mIntPipeMDP, &mIntCompletionInfo, NULL );
        return DFW_ERR_NO_ERROR;
    }
    return DFW_ERR_INTERNAL_ERROR;
}

int TIUMPDevice::StopIntPolling()
{
    if (mIntPipeStarted && mIntPipe) {
        mIntPipeStarted = false;
        mIntPipe->Abort();
		mIntPipe->ClearPipeStall(true);
        return DFW_ERR_NO_ERROR;
    }
    return DFW_ERR_INTERNAL_ERROR;
}


bool TIUMPDevice::IsFWLoaded(void)
{
    //Can we talk to FW? If yes, the FW is already in the device
    tDevFwVersion fwvers;
	if (GetFWVersion(&fwvers)==DFW_ERR_NO_ERROR) {
		DLOG("Firmware version: %d.%d - %d.%d\n",fwvers.bAppRevMajor, fwvers.bAppRevMinor,fwvers.bDfwRevMajor, fwvers.bDfwRevMinor);
		return true;
	} else {
		return false;
	}
}

int TIUMPDevice::UploadFirmware(void)
{
    UInt8 *fwdata;
    UInt32 fwdatasize;
    
    if (mChipID==kChip3410) {
        fwdata = (UInt8*)frmware3410;
        fwdatasize = sizeof(frmware3410);
    } else {
        fwdata = (UInt8*)frmware5052;
        fwdatasize = sizeof(frmware5052);
    }
    
    IOReturn err = mUSBDevice->SetConfiguration(mService, 1, false);
    if (err!=kIOReturnSuccess) {
        DLOG("FWUpload - Set Configuration failed!!!\n");
        return DFW_ERR_UNRECOGNIZED_COMMAND;
    }
    
    //we need to find interface and pipe for FW upload
    IOUSBFindInterfaceRequest intreq;
    
    intreq.bInterfaceClass = 255;
    intreq.bInterfaceSubClass = 0;
    intreq.bInterfaceProtocol = 0;
    intreq.bAlternateSetting = 0;
        
    IOUSBInterface *intf = mUSBDevice->FindNextInterface(NULL, &intreq);
    if (intf == NULL) {
        DLOG("FWUpload - Interface is NULL!!!\n");
        return DFW_ERR_UNRECOGNIZED_COMMAND;
    }
    
    intf->open(mService);
    
    IOUSBPipe *pipe = intf->GetPipeObj(0);
    if (pipe == NULL || pipe->GetDirection()!=kUSBOut) {
        DLOG("FWUpload - pipe is NULL or not correct!!!\n");
        return DFW_ERR_UNRECOGNIZED_COMMAND;
    }
        
    
    //prepare upload buffer
    UInt8 *fwbuffer = (u_char *)IOMalloc(fwdatasize+4);
   
    UInt8 *tmpptr=fwbuffer + 3;
    
    UInt8 chcksm = 0;

    for (UInt32 i=0;i<fwdatasize;i++) {
        unsigned char ch = (fwdata)[i];
        
        tmpptr[i]=ch;
        
        chcksm+=ch;
    }
				
    fwbuffer[0] = fwdatasize&0x00FF;
    fwbuffer[1] = (fwdatasize>>8)&0x00FF;
    fwbuffer[2] = chcksm;
    
    fwdatasize+=3;
            
    
    //upload firmware
    err = pipe->Write(IOMemoryDescriptor::withAddress(fwbuffer,fwdatasize,kIODirectionOut));
        
    if (err != kIOReturnSuccess) {
        DLOG("FWUpload - fw writting was unsuccessful - %4X!!!\n",err);
        return DFW_ERR_UNRECOGNIZED_COMMAND;
    }
    
    IOFree(fwbuffer, fwdatasize+4);
    
    //give bus a time to update itself
    
    intf->close(mService);
    
    //reset device to be sure.
    mUSBDevice->ResetDevice();
    
    IOSleep(250);
    
    mUSBDevice->ReEnumerateDevice(0);
        
    return DFW_ERR_NO_ERROR;
}

bool TIUMPDevice::createSuffix(int portidx, unsigned char *sufKey)
{
    
    IOReturn	rc;
    UInt8	serBuf[12];		// arbitrary size > 8
    OSNumber	*location;
    UInt32	locVal;
    SInt16	i, sig = 0;
    UInt8	indx;
    bool	keyOK = false;			
		
    indx = mUSBDevice->GetSerialNumberStringIndex();	
    if (indx != 0)
    {	
		// Generate suffix key based on the serial number string (if reasonable <= 8 and > 0)	
		
        rc = mUSBDevice->GetStringDescriptor(indx, (char *)&serBuf, sizeof(serBuf));
        if (!rc)
        {
            if ((strlen((char *)&serBuf) < 9) && (strlen((char *)&serBuf) > 0))
            {
                strncpy((char *)sufKey, (const char *)&serBuf, 9);
                sig = strlen((char *)sufKey);
                keyOK = true;
            }			
        }
    }
	
    if (!keyOK)
    {
		// Generate suffix key based on the location property tag
		
        location = (OSNumber *)mUSBDevice->getProperty(kUSBDevicePropertyLocationID);
        if (location)
        {
            locVal = location->unsigned32BitValue();
			snprintf((char *)sufKey, (sizeof(locVal)*2)+1, "%x", locVal);
			sig = strlen((const char *)sufKey)-1;
			for (i=sig; i>=0; i--)
			{
				if (sufKey[i] != '0')
				{
					break;
				}
			}
			sig = i + 1;
            keyOK = true;
        }
    }
    
	// Make it unique just in case there's more than one CDC configuration on this device
    
    if (keyOK)
    {
        sufKey[sig] = Asciify((UInt8)portidx >> 4);
		if (sufKey[sig] != '0')
            sig++;	
        sufKey[sig++] = Asciify((UInt8)portidx);
        sufKey[sig] = 0x00;
    }
	
    return keyOK;
	
}/* end createSuffix */



int TIUMPDevice::EnumeratePorts(void)
{
    //we should be here only when FW is in device, so let's start with this assumption

    IOReturn err;
	
	unsigned char rname[20];
	const char  *suffix = (const char *)&rname;
	
	unsigned char prodName[64];
	const char *prod = (const char *)&prodName;

    
    struct {
        UInt8       num;
        IOUSBPipe   *inpipe;
        IOUSBPipe   *outpipe;
        int         id;
    } endpoints[] = {
    {0x1, NULL, NULL, UMPM_UART1_PORT},
    {0x2, NULL, NULL, UMPM_UART2_PORT}
    }; 
    
   int endpnum = sizeof(endpoints)/sizeof(*endpoints);
    
    //on 3410 chip FW, configuration 2 is added, on 5052 FW it stays as value 1
    if (mChipID==kChip3410) {
        err = mUSBDevice->SetConfiguration(mService, 2, false);
    } else {
        err = mUSBDevice->SetConfiguration(mService, 2, false);
    }
   
    if (err!=kIOReturnSuccess) {
        DLOG("Enumerate - Set Configuration failed!!!\n");
        return DFW_ERR_UNRECOGNIZED_COMMAND;
    }
    
    //we need to find interface and pipe for FW upload
    IOUSBFindInterfaceRequest intreq;
    
    intreq.bInterfaceClass = 255;
    intreq.bInterfaceSubClass = 0;
    intreq.bInterfaceProtocol = 0;
    intreq.bAlternateSetting = 0;
    
    IOUSBInterface *intf = mUSBDevice->FindNextInterface(NULL, &intreq);
    if (intf == NULL) {
        DLOG("Enumerate - Interface is NULL!!!\n");
        return DFW_ERR_UNRECOGNIZED_COMMAND;
    }
    
    intf->open(mService);
    
    int pipeNum = intf->GetNumEndpoints();
    
    for (int i=0;i<pipeNum;i++) {
        IOUSBPipe *pipe = intf->GetPipeObj(i);
        
        switch(pipe->GetType()) {
            case kUSBInterrupt:
                if (mIntPipe==NULL && pipe->GetDirection()==kUSBIn) {
					DLOG("kUSBInterrupt: %08X\n", pipe->GetEndpointNumber()); 
                    mIntPipe=pipe;
                    pipe->retain();
                }
                break;
            case kUSBBulk:
                for (int j=0;j<endpnum;j++) {
                    if (endpoints[j].num==pipe->GetEndpointNumber()) {
                        if (pipe->GetDirection()==kUSBOut) {
                            endpoints[j].outpipe=pipe;
                        } else {
                            endpoints[j].inpipe=pipe;
                        }
                    };
                }
        }
    }
	
	intf->close(mService);
    
    mListAllocSize = endpnum*sizeof(IOUSBPipe*);
    
    mPortList = (TIUMPPort**)IOMalloc(mListAllocSize);
    if (mPortList == NULL) {
        DLOG("Enumerate - mPortList is NULL!!!\n");
        return DFW_ERR_UNRECOGNIZED_COMMAND;
    }
    
    for (int i=0;i<endpnum;i++) {
        if (endpoints[i].inpipe!=NULL && endpoints[i].outpipe!=NULL) {
            DLOG("Endpoint: %d\n",endpoints[i].num);
            
            TIUMPPort *port=new TIUMPPort(this, endpoints[i].inpipe, endpoints[i].outpipe, endpoints[i].id);
            if (port!=NULL) {
                mPortList[mPortNum++]=port;
				
				IORS232SerialStreamSync *rs232Stream = new IORS232SerialStreamSync;
				if (!rs232Stream)
					return false;

				// Either we attach and should get rid of our reference
				// or we have fail in which case we should get rid our reference.
				bool ret = (rs232Stream->init(0, port) && rs232Stream->attach(mService));
				rs232Stream->release();

				if (ret) 
				{
					// Make sure we copy the PORTNAME up as well.
					OSString *basename = OSDynamicCast(OSString, mService->getProperty(kIOTTYBaseNameKey));
					if (basename == NULL)
					{
						DLOG("no kIOTTYBaseNameKey\n");
					}
					else
					{
						const char *str = basename->getCStringNoCopy();
					
						if (str == NULL)
						{
							DLOG("no CString\n");
						}
						else
						{
							/*char newname[256];
							
							if (i==0)
								sprintf(newname,"%s",str);
							else
								sprintf(newname,"%s-%d",str,i);*/

							rs232Stream->setProperty(kIOTTYBaseNameKey, str);
														
							if (createSuffix(i,(unsigned char *)suffix))
							{
								DLOG("%s", (char *)suffix);DLOG("\n");
								rs232Stream->setProperty(kIOTTYSuffixKey, suffix);
							}
							
						}
					}

					rs232Stream->registerService();
										
					port->SetStream(rs232Stream);
				}
			}
        }
    }
    
    return DFW_ERR_NO_ERROR;
}


int TIUMPDevice::CallFW(CommDirT inDirection, TIUMPPort *inPort, int inSelector, int inValue, void *ioBuffer, UInt32 &ioBufSize)
{	
	IOUSBDevRequestDesc devrequest;
  
	if (mTerminate) {
		DLOG("we've been terminated\n");
		return DFW_ERR_PORT_NOT_OPENED;
	}
	  
	devrequest.bmRequestType = USBmakebmRequestType((inDirection==kComDirectionIn)?kUSBIn:kUSBOut, kUSBVendor, kUSBDevice);
    devrequest.bRequest = inSelector;
    devrequest.wValue = inValue;
    devrequest.wIndex = (inPort)?inPort->GetPortID():0;
	
	mControlPipeOutMDP->setLength(ioBufSize);
	mControlPipeInMDP->setLength(ioBufSize);
	
	mControlPipeOutBuffer = (UInt8*)mControlPipeOutMDP->getBytesNoCopy();
	mControlPipeInBuffer = (UInt8*)mControlPipeInMDP->getBytesNoCopy();
	
	if (ioBuffer == NULL)
	{
		devrequest.pData = NULL;
		devrequest.wLength = 0;
	}
	else
	{
		devrequest.pData = (inDirection==kComDirectionIn)?mControlPipeInMDP:mControlPipeOutMDP;
		devrequest.wLength = ioBufSize;

		if (inDirection==kComDirectionOut) {
			for (UInt32 i=0;i<ioBufSize;i++) {
				mControlPipeOutBuffer[i]=((UInt8*)ioBuffer)[i];
			}
		}
		else
		{
			devrequest.wLength += sizeof(tDevFwCmdHeader);
			mControlPipeInMDP->setLength(devrequest.wLength);
			mControlPipeInBuffer = (UInt8*)mControlPipeInMDP->getBytesNoCopy();
		}
	}

	DLOG ("Before Device request\n");

	IOReturn err = mUSBDevice->DeviceRequest(&devrequest,250,250);
		
	DLOG ("Device request - error %08X, wLenDone %d\n",err,devrequest.wLenDone);
	
	if (inDirection==kComDirectionIn && ioBuffer) {
		for (UInt32 i=0;i<devrequest.wLenDone && i < ioBufSize;i++) {
			((UInt8*)ioBuffer)[i]=mControlPipeInBuffer[i+sizeof(tDevFwCmdHeader)];
		}
		devrequest.wLenDone -= sizeof(tDevFwCmdHeader);
	}
		
	if (ioBuffer)
		DEVLogData(kUSBOut,ioBufSize,(char*)ioBuffer);
    
    if (err!=kIOReturnSuccess) {
        tDevFwCmdHeader commresp;
        
		DLOG("devrequest error!\n");
		
        devrequest.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBVendor, kUSBDevice);
		mControlPipeInMDP->setLength(sizeof(commresp));
		mControlPipeInBuffer = (UInt8*)mControlPipeInMDP->getBytesNoCopy();
		
        devrequest.pData = mControlPipeInMDP;
        devrequest.wLength = sizeof(commresp);
        IOReturn err = mUSBDevice->DeviceRequest(&devrequest,250,250);
        if (err!=kIOReturnSuccess) {
            return DFW_ERR_UNRECOGNIZED_COMMAND;
        } else {
            return ((ptDevFwCmdHeader)mControlPipeInBuffer)->bErrorCode;
        }
    } else {
        ioBufSize = devrequest.wLenDone;
        return DFW_ERR_NO_ERROR;
    }
}

int TIUMPDevice::GetFWVersion(ptDevFwVersion outVersion)
{
	
    UInt32 size = sizeof(*outVersion);
    int err = CallFW(kComDirectionIn, NULL, DFW_CMD_GET_VERSION, 0, outVersion, size);
	
	FixFWVersionEndianess(outVersion);
	
	return err;
}

int TIUMPDevice::OpenPort(TIUMPPort *inPort)
{
    UInt32 size=0;
    return CallFW(kComDirectionOut, inPort, UMPC_OPEN_PORT, 0x0089, NULL, size);
}

int TIUMPDevice::ClosePort(TIUMPPort *inPort)
{
    UInt32 size=0;
    return CallFW(kComDirectionOut, inPort, UMPC_CLOSE_PORT, 0, NULL, size);
}

int TIUMPDevice::StartPort(TIUMPPort *inPort)
{
    UInt32 size=0;
    return CallFW(kComDirectionOut, inPort, UMPC_START_PORT, 0, NULL, size);
}

int TIUMPDevice::StopPort(TIUMPPort *inPort)
{
    UInt32 size=0;
    return CallFW(kComDirectionOut, inPort, UMPC_STOP_PORT, 0, NULL, size);
}

int TIUMPDevice::GetPortConfig(TIUMPPort *inPort, ptUmpUartConfig outConfig)
{
    UInt32 size = sizeof(*outConfig);
    int err = CallFW(kComDirectionIn, inPort, UMPC_GET_CONFIG, 0, outConfig, size);
	
	FixConfigEndianess(outConfig);
	
	return err;
}

int TIUMPDevice::SetPortConfig(TIUMPPort *inPort, ptUmpUartConfig inConfig)
{
    UInt32 size = sizeof(*inConfig);

	tUmpUartConfig tmpConfig = *inConfig;
	
	//trust no one
	tmpConfig.bUartMode = UMP_UART_232MODE;
	tmpConfig.wFlags |= /*UMP_MASK_UART_FLAGS_RECEIVE_MS_INT | */UMP_MASK_UART_FLAGS_AUTO_START_ON_ERR;
	
	FixConfigEndianess(&tmpConfig);
		
    return CallFW(kComDirectionOut, inPort, UMPC_SET_CONFIG, 0, &tmpConfig, size);
}

int TIUMPDevice::GetPortStatus(TIUMPPort *inPort, ptUmpPortStatus outStatus)
{
	UInt32 size = sizeof(tUmpPortStatus);
    return CallFW(kComDirectionIn, inPort, UMPC_GET_PORT_STATUS, 0, outStatus, size);
}

#ifdef __LITTLE_ENDIAN__
#define FLIP_WORD_(x) ((WORD)((((WORD)x >> 8) & 0xFF) | (((WORD)x << 8) & 0xFF00)))
#else
#define FLIP_WORD_(x) (x)
#endif

void TIUMPDevice::FixConfigEndianess(ptUmpUartConfig ioConfig)
{
	ioConfig->wBaudRate = FLIP_WORD_(ioConfig->wBaudRate);
	ioConfig->wFlags = FLIP_WORD_(ioConfig->wFlags);
}

void TIUMPDevice::FixFWVersionEndianess(ptDevFwVersion ioFWVer)
{
	ioFWVer->wDfwCapabil = FLIP_WORD_(ioFWVer->wDfwCapabil);
	ioFWVer->wMaxDataLength = FLIP_WORD_(ioFWVer->wMaxDataLength);
}


#define ENDTABLE 0xffffffff	

typedef struct 
{
	UInt32 rate;							// The actual baud rate setting.
	UInt16 divisor;						// The device divisor.
} 	BAUD_TABLE, *PBAUD_TABLE;			// baud rate table and pointer defs.


BAUD_TABLE brTable3410[] =
{
	{ 50,           UMP_3410_UART_50   },
    { 75,           UMP_3410_UART_75   },
    { 135,          UMP_3410_UART_135  },
    { 150,          UMP_3410_UART_150  },
    { 110,          UMP_3410_UART_110  },
    { 300,          UMP_3410_UART_300  },
    { 600,          UMP_3410_UART_600  },
    { 1200,         UMP_3410_UART_1200 },
    { 1800,         UMP_3410_UART_1800 },
    { 2000,         UMP_3410_UART_2000 },
    { 2400,         UMP_3410_UART_2400 },
    { 3600,         UMP_3410_UART_3600 },
    { 4800,         UMP_3410_UART_4800 },
    { 7200,         UMP_3410_UART_7200 },
    { 9600,         UMP_3410_UART_9600 },
    { 14400,		0x40			   },
    { 19200,        UMP_3410_UART_19K  },
    { 38400,        UMP_3410_UART_38K  },
    { 57600,        UMP_3410_UART_57K  },
    { 115200,       UMP_3410_UART_115K },
    { 230400,       UMP_3410_UART_230K },
    { 460800,       UMP_3410_UART_460K },
	{ 921600,       UMP_3410_UART_920K },
	{ ENDTABLE,		0 }
};

BAUD_TABLE brTable5x52[] =
{
	{ 50,           UMP_5x52_UART_50   },
    { 75,           UMP_5x52_UART_75   },
    { 135,          UMP_5x52_UART_135  },
    { 150,          UMP_5x52_UART_150  },
    { 110,          UMP_5x52_UART_110  },
    { 300,          UMP_5x52_UART_300  },
    { 600,          UMP_5x52_UART_600  },
    { 1200,         UMP_5x52_UART_1200 },
    { 1800,         UMP_5x52_UART_1800 },
    { 2000,         UMP_5x52_UART_2000 },
    { 2400,         UMP_5x52_UART_2400 },
    { 3600,         UMP_5x52_UART_3600 },
    { 4800,         UMP_5x52_UART_4800 },
    { 7200,         UMP_5x52_UART_7200 },
    { 9600,         UMP_5x52_UART_9600 },
    { 14400,		0x20			   },
    { 19200,        UMP_5x52_UART_19K  },
    { 38400,        UMP_5x52_UART_38K  },
    { 57600,        UMP_5x52_UART_57K  },
    { 115200,       UMP_5x52_UART_115K },
    { 230400,       UMP_5x52_UART_230K },
    { 460800,       UMP_5x52_UART_460K },
	{ ENDTABLE,		0 }
};

WORD TIUMPDevice::EncodeBaudRate(UInt32 baudRate)
{
	int ii;
	
	BAUD_TABLE *pBrTable;
	
	if (mChipID == kChip3410) 
		pBrTable = brTable3410;
	else 
		pBrTable = brTable5x52;
	
	for ( ii = 0; pBrTable[ii].rate!= ENDTABLE; ii++ )
	{
		if ( pBrTable[ii].rate == baudRate )
		{
			return pBrTable[ii].divisor;
		}
	}
	
	return UMP_3410_UART_9600;
	
	if (mChipID == kChip3410)
		return UMP_3410_UART_9600;//return 14.76923077E06/(baudRate*16);
	else
		return UMP_5x52_UART_9600;//return 7.384615E06/(baudRate*16);
}


/*----------------------------------------------------------------------------------------*/

TIUMPPort::TIUMPPort(TIUMPDevice *inUMPDevice, IOUSBPipe *inPipeIn, IOUSBPipe *inPipeOut, int inPortID)
{
	DLOG("TIUMPPort_c\n");

    mPipeIn = inPipeIn;
    mPipeIn->retain();
    mPipeOut = inPipeOut;
    mPipeOut->retain();
    mPortID = inPortID;
    mUMPDevice = inUMPDevice;
    mRequestLock = IORecursiveLockAlloc();   // init lock used to protect code on MP
    mWatchLock = IOLockAlloc();
	mAcquired = false;
    mReadActive = false;
    mWriteActive = false;
    mWatchStateMask = 0;
	mOSPortState = 0;
	mOSFlowControlState = 0;
	mRS232Stream = NULL;
}

TIUMPPort::~TIUMPPort()
{
	DLOG("~TIUMPPort\n");
	
	if (IsAcquired())
		Release();
    mPipeIn->release();
    mPipeOut->release();
    if (mRequestLock) {
        IORecursiveLockFree(mRequestLock);
        mRequestLock = NULL;
    }
    if (mWatchLock) {
        IOLockFree(mWatchLock);
        mWatchLock = NULL;
    }
}

IOReturn TIUMPPort::Acquire()
{
    DLOG("Acquire\n");
    
   // ptUmpPortStatus outStatus;
   
	SetDefaults();
	
	IOReturn rtn;

	rtn = mUMPDevice->StartIntPolling();
	if (rtn!=kIOReturnSuccess)
		return rtn;

    if (mUMPDevice->OpenPort(this) != DFW_ERR_NO_ERROR)
		return kIOReturnError;
	    	    
	ReloadStatus();
    ReloadConfig();

    mAcquired = true;
	
	ChangeState(PD_S_ACQUIRED | DEFAULT_STATE,STATE_ALL);
	
    return kIOReturnSuccess;
}

IOReturn TIUMPPort::Release()
{
    DLOG("Release\n");
	
	if (ReadPortState() & PD_S_ACTIVE)
		Stop();

    mUMPDevice->ClosePort(this);
		
    mAcquired = false;
    
    ChangeState(0,STATE_ALL);
	
    return kIOReturnSuccess;
}

bool TIUMPPort::IsAcquired()
{
    return mAcquired;
}

IOReturn TIUMPPort::Start(void)
{
	DLOG("StartPort\n");
    
	if (ReadPortState() & PD_S_ACTIVE)
		return kIOReturnSuccess;
	
    AllocateRingBuffer(&mTX, mTXStats.BufferSize);
    AllocateRingBuffer(&mRX, mRXStats.BufferSize);
	
	mUMPDevice->StartPort(this);
	ReloadStatus();
    AllocateResources();
	StartPipes();
	ChangeState(PD_S_ACTIVE,PD_S_ACTIVE);

	return kIOReturnSuccess;
}

IOReturn TIUMPPort::Stop(void)
{
	DLOG("StopPort\n");

	if (!(ReadPortState() & PD_S_ACTIVE))
		return kIOReturnNotOpen;

	mUMPDevice->StopPort(this);
	DLOG("After mUMPDevice->StopPort(this);\n");
	StopPipes();
	DLOG("After StopPipes();\n");
    FreeResources();
	DLOG("After FreeResources();\n");
	
	FreeRingBuffer(&mTX);
	DLOG("After FreeRingBuffer(&mTX);\n");
	FreeRingBuffer(&mRX);
	DLOG("After FreeRingBuffer(&mRX);\n");
	
	ChangeState(0,PD_S_ACTIVE);
	DLOG("After ChangeState(0,PD_S_ACTIVE);\n");
	
	return kIOReturnSuccess;
}


IOReturn TIUMPPort::AllocateResources( void )
{        
    mPipeInMDP = IOBufferMemoryDescriptor::withCapacity( USBLapPayLoad, kIODirectionIn );
    mPipeInMDP->setLength( USBLapPayLoad );
    mPipeInBuffer = (UInt8*)mPipeInMDP->getBytesNoCopy();
    
    // Allocate Memory Descriptor Pointer with memory for the data-out bulk pipe:
    
    mPipeOutMDP = IOBufferMemoryDescriptor::withCapacity( MAX_BLOCK_SIZE, kIODirectionOut );
    mPipeOutMDP->setLength( MAX_BLOCK_SIZE );
    mPipeOutBuffer = (UInt8*)mPipeOutMDP->getBytesNoCopy();

    mReadCompletionInfo.target  = this;
    mReadCompletionInfo.action  = DataReadComplete;
    mReadCompletionInfo.parameter   = NULL;
    
    mWriteCompletionInfo.target = this;
    mWriteCompletionInfo.action = DataWriteComplete;
    mWriteCompletionInfo.parameter  = NULL;

    return kIOReturnSuccess;
} 

IOReturn TIUMPPort::FreeResources(void)
{
	if (mPipeInMDP) {
		mPipeInMDP->release();
		mPipeInMDP = 0;
	}
	
	if (mPipeOutMDP) {
		mPipeOutMDP->release();
		mPipeOutMDP = 0;
	}

	return kIOReturnSuccess;
}

IOReturn TIUMPPort::StartPipes( void )
{
    IOReturn rtn;
    	
    //rtn = mPipeIn->Read(mPipeInMDP, 1000, 1000, &mReadCompletionInfo, NULL );
	rtn = mPipeIn->Read(mPipeInMDP, &mReadCompletionInfo, NULL );
    if (rtn!=kIOReturnSuccess) {
        return rtn;
    }
    
    mReadActive = true;     // remember if we did a read

    // is this really referenced by anyone??
    
    return kIOReturnSuccess;
}

void TIUMPPort::StopPipes()
{
	mUMPDevice->StopIntPolling();

	if (mPipeIn)
	{
		mPipeIn->Abort();
		mPipeIn->ClearPipeStall(true);
	}
    if (mPipeOut)
	{
		mPipeOut->Abort();
		mPipeOut->ClearPipeStall(true);
	}
}

void TIUMPPort::DataReadComplete( void *obj, void *param, IOReturn rc, UInt32 remaining )
{
    TIUMPPort       *me = (TIUMPPort*)obj;
    UInt16          dtlength;
    IOReturn        ior = kIOReturnSuccess;
        
    DLOG("DataReadComplete - error %08X\n", rc);
	
	if(!me->mReadActive)
        return;
		
	me->mReadActive = false;
    
    if ( rc == kIOReturnSuccess )   /* If operation returned ok:    */
    {
		//me->mReadActive = false;
		dtlength = USBLapPayLoad - remaining;
		
		DEVLogData(kUSBIn, dtlength, (char*)me->mPipeInMDP->getBytesNoCopy());
        
		if (dtlength>0)
			me->Add_RXBytes((UInt8*)me->mPipeInMDP->getBytesNoCopy(),dtlength);
			
		DLOG("DataReadComplete - rereading\n");
            		
		ior = me->mPipeIn->Read( me->mPipeInMDP, &me->mReadCompletionInfo, NULL );
		
		if ( ior == kIOReturnSuccess )
        {
			me->mReadActive = true;
			me->CheckQueues();
            return;
        }
    }
}

void TIUMPPort::DataWriteComplete( void *obj, void *param, IOReturn rc, UInt32 remaining )
{
    TIUMPPort  *me = (TIUMPPort*)obj;
     
	DLOG("DataWriteComplete - error %08X, remaining %d\n", rc, remaining);
		      
    if (!me->mWriteActive)
        return;
    
    me->mWriteActive = false;
	
	// We potentially removed a bunch of stuff from the
	// queue, so see if we can free some thread(s)
	// to enqueue more stuff.
		
	me->CheckQueues();
	
	me->SetUpTransmit();
	
	if (!me->mWriteActive)
		me->ChangeState( 0, PD_S_TX_BUSY );
	        
    // in a transmit complete, but need to manually transmit a zero-length packet
    // if it's a multiple of the max usb packet size for the bulk-out pipe (64 bytes)
    if ( rc == kIOReturnSuccess )   /* If operation returned ok:    */
    {
    }
}


TIUMPPort::QueueStatus TIUMPPort::AddBytetoQueue( TIUMPPort::CirQueue *Queue, char Value )
{
    /* Check to see if there is space by comparing the next pointer,    */
    /* with the last, If they match we are either Empty or full, so     */
    /* check the InQueue of being zero.                 */

    IORecursiveLockLock( mRequestLock );

    if ( (Queue->NextChar == Queue->LastChar) && Queue->InQueue ) {
		IORecursiveLockUnlock( mRequestLock);
		return queueFull;
    }

    *Queue->NextChar++ = Value;
    Queue->InQueue++;

	/* Check to see if we need to wrap the pointer. */
	
    if ( Queue->NextChar >= Queue->End )
		Queue->NextChar =  Queue->Start;

    IORecursiveLockUnlock( mRequestLock);
    return queueNoError;
}/* end AddBytetoQueue */


TIUMPPort::QueueStatus TIUMPPort::GetBytefromQueue( TIUMPPort::CirQueue *Queue, UInt8 *Value )
{
    IORecursiveLockLock( mRequestLock );

	/* Check to see if the queue has something in it.   */
	
    if ( (Queue->NextChar == Queue->LastChar) && !Queue->InQueue ) {
		IORecursiveLockUnlock(mRequestLock);
		return queueEmpty;
    }

    *Value = *Queue->LastChar++;
    Queue->InQueue--;

	/* Check to see if we need to wrap the pointer. */
	
    if ( Queue->LastChar >= Queue->End )
	Queue->LastChar =  Queue->Start;

    IORecursiveLockUnlock(mRequestLock);
    return queueNoError;
}/* end GetBytefromQueue */

TIUMPPort::QueueStatus TIUMPPort::InitQueue( TIUMPPort::CirQueue *Queue, UInt8 *Buffer, size_t Size )
{
    Queue->Start    = Buffer;
    Queue->End      = (UInt8*)((size_t)Buffer + Size);
    Queue->Size     = Size;
    Queue->NextChar = Buffer;
    Queue->LastChar = Buffer;
    Queue->InQueue  = 0;

    IOSleep( 1 );
    
    return queueNoError ;
    
}/* end InitQueue */


TIUMPPort::QueueStatus TIUMPPort::CloseQueue( TIUMPPort::CirQueue *Queue )
{

    Queue->Start    = 0;
    Queue->End      = 0;
    Queue->NextChar = 0;
    Queue->LastChar = 0;
    Queue->Size     = 0;

    return queueNoError;
    
}/* end CloseQueue */


size_t TIUMPPort::AddtoQueue( TIUMPPort::CirQueue *Queue, UInt8 *Buffer, size_t Size )
{
    size_t      BytesWritten = 0;

    while ( FreeSpaceinQueue( Queue ) && (Size > BytesWritten) )
    {
		AddBytetoQueue( Queue, *Buffer++ );
		BytesWritten++;
    }

    return BytesWritten;
    
}/* end AddtoQueue */


size_t TIUMPPort::RemovefromQueue( TIUMPPort::CirQueue *Queue, UInt8 *Buffer, size_t MaxSize )
{
    size_t      BytesReceived = 0;
    UInt8       Value;
    
    while( (MaxSize > BytesReceived) && (GetBytefromQueue(Queue, &Value) == queueNoError) ) 
    {
	*Buffer++ = Value;
	BytesReceived++;
    }/* end while */

    return BytesReceived;
    
}/* end RemovefromQueue */

size_t TIUMPPort::FreeSpaceinQueue( TIUMPPort::CirQueue *Queue )
{
    size_t  retVal = 0;

    IORecursiveLockLock( mRequestLock );

    retVal = Queue->Size - Queue->InQueue;
    
    IORecursiveLockUnlock(mRequestLock);
	
	return retVal;
}/* end FreeSpaceinQueue */

size_t TIUMPPort::UsedSpaceinQueue( TIUMPPort::CirQueue *Queue )
{
    return Queue->InQueue;
}/* end UsedSpaceinQueue */

size_t TIUMPPort::GetQueueSize( TIUMPPort::CirQueue *Queue )
{
    return Queue->Size;
    
}/* end GetQueueSize */


void TIUMPPort::CheckQueues()
{
    unsigned long   Used;
    unsigned long   Free;
    unsigned long   QueuingState;
    unsigned long   DeltaState;
    
    // Initialise the QueueState with the current state.
    QueuingState = ReadPortState();
	
	DLOG("QueuingState - %08lX\n", QueuingState);
    
    /* Check to see if there is anything in the Transmit buffer. */
    Used = UsedSpaceinQueue( &mTX );
    Free = FreeSpaceinQueue( &mTX );
    //  ELG( Free, Used, 'CkQs', "CheckQueues" );
    if ( Free == 0 )
    {
		QueuingState |=  PD_S_TXQ_FULL;
		QueuingState &= ~PD_S_TXQ_EMPTY;
    }
    else if ( Used == 0 )
    {
		QueuingState &= ~PD_S_TXQ_FULL;
		QueuingState |=  PD_S_TXQ_EMPTY;
    }
    else
    {
		QueuingState &= ~PD_S_TXQ_FULL;
		QueuingState &= ~PD_S_TXQ_EMPTY;
    }
    
    /* Check to see if we are below the low water mark. */
    if ( Used < mTXStats.LowWater )
        QueuingState |=  PD_S_TXQ_LOW_WATER;
    else QueuingState &= ~PD_S_TXQ_LOW_WATER;
    
    if ( Used > mTXStats.HighWater )
        QueuingState |= PD_S_TXQ_HIGH_WATER;
    else QueuingState &= ~PD_S_TXQ_HIGH_WATER;
    
    
    /* Check to see if there is anything in the Receive buffer. */
    Used = UsedSpaceinQueue( &mRX );
    Free = FreeSpaceinQueue( &mRX );
    
    if ( Free == 0 )
    {
		QueuingState |= PD_S_RXQ_FULL;
		QueuingState &= ~PD_S_RXQ_EMPTY;
    }
    else if ( Used == 0 )
    {
		QueuingState &= ~PD_S_RXQ_FULL;
		QueuingState |= PD_S_RXQ_EMPTY;
    }
    else
    {
		QueuingState &= ~PD_S_RXQ_FULL;
		QueuingState &= ~PD_S_RXQ_EMPTY;
    }
    
    /* Check to see if we are below the low water mark. */
    if ( Used < mRXStats.LowWater )
        QueuingState |= PD_S_RXQ_LOW_WATER;
    else QueuingState &= ~PD_S_RXQ_LOW_WATER;
    
    if ( Used > mRXStats.HighWater )
        QueuingState |= PD_S_RXQ_HIGH_WATER;
    else QueuingState &= ~PD_S_RXQ_HIGH_WATER;
    
    /* Figure out what has changed to get mask.*/
    DeltaState = QueuingState ^ ReadPortState();
	
	//if (DeltaState != 0)
		ChangeState( QueuingState, DeltaState );

}/* end CheckQueues */


void TIUMPPort::Add_RXBytes( UInt8 *Buffer, size_t Size )
{
    DLOG("Add_RXBytes %ld\n", Size);
    
    AddtoQueue(&mRX, Buffer, Size);
    CheckQueues();
}

void TIUMPPort::SetDefaults(void)
{
    //  port->RXStats.BufferSize    = BUFFER_SIZE_DEFAULT;
    mRXStats.BufferSize    = kMaxCirBufferSize;
    //  port->RXStats.HighWater     = port->RXStats.BufferSize - (DATA_BUFF_SIZE*2);
    mRXStats.HighWater     = (mRXStats.BufferSize << 1) / 3;
    mRXStats.LowWater      = mRXStats.HighWater >> 1;
    
    //  port->TXStats.BufferSize    = BUFFER_SIZE_DEFAULT;
    mTXStats.BufferSize    = kMaxCirBufferSize;
    mTXStats.HighWater     = (mRXStats.BufferSize << 1) / 3;
    mTXStats.LowWater      = mRXStats.HighWater >> 1;
	
	SetOSConfigDefaults();
}

UInt32 TIUMPPort::ReadPortState()
{
    UInt32              returnState;
        
    IORecursiveLockLock( mRequestLock );
    
    returnState = mOSPortState;
        
    IORecursiveLockUnlock( mRequestLock);
        
    return returnState;
    
}

void TIUMPPort::ChangeState( UInt32 state, UInt32 mask )
{
    UInt32              delta;
    
    //  ELG( state, mask, 'chSt', "changeState" );
	
	if (mask == 0)
		return;
	
	 DLOG("Change state %08X, %08X\n", state, mask);
    
    IORecursiveLockLock(mRequestLock);
    state = (mOSPortState & ~mask) | (state & mask); // compute the new state
    delta = state ^ mOSPortState;                    // keep a copy of the diffs
    mOSPortState = state;
	
	if (delta & PD_RS232_S_DTR) {
		TIUMPPort::OSConfig cfg;
		DLOG("*** DTR Changed\n");
		SetOSConfig(cfg, kMskState);
	}
	
	if (delta & PD_RS232_S_RTS) {
		TIUMPPort::OSConfig cfg;
		DLOG("*** RTS Changed\n");
		SetOSConfig(cfg, kMskState);
	}
	
	if (delta & PD_RS232_S_CTS) {
		DLOG("*** CTS Changed\n");
	}
    
	//IORecursiveLockUnlock(mRequestLock);    
    
	// Wake up all threads asleep on WatchStateMask
    if ( delta & mWatchStateMask ) {
		thread_wakeup_with_result( &mWatchStateMask, IsAcquired()?THREAD_RESTART:-1 );
    }
    
    IORecursiveLockUnlock(mRequestLock);    
}

IOReturn TIUMPPort::WatchState(UInt32 *state, UInt32 mask)
{
    unsigned            watchState, foundStates;
    bool                autoActiveBit   = false;
    IOReturn            rtn             = kIOReturnSuccess;
    
	DLOG("WatchState - state %08X mask %08X\n",*state, mask);   	
	
	    //    ELG( mask, *state, 'wsta', "privateWatchState" );
    
    watchState = *state;
    IORecursiveLockLock( mRequestLock);
    
    // hack to get around problem with carrier detection
    
    if ( *state | 0x40 )    /// mlj ??? PD_S_RXQ_FULL?
    {
		mOSPortState |= 0x40;
    }
    
    if ( !(mask & (PD_S_ACQUIRED | PD_S_ACTIVE)) )
    {
		watchState &= ~PD_S_ACTIVE; // Check for low PD_S_ACTIVE
		mask       |=  PD_S_ACTIVE; // Register interest in PD_S_ACTIVE bit
		autoActiveBit = true;
    }
    
    for (;;)
    {
        // Check port state for any interesting bits with watchState value
        // NB. the '^ ~' is a XNOR and tests for equality of bits.
        
		foundStates = (watchState ^ ~mOSPortState) & mask;
			
		if ( foundStates )
		{
			*state = mOSPortState;
			if ( autoActiveBit && (foundStates & PD_S_ACTIVE) )
			{
				rtn = kIOReturnIOError;
			} else {
				rtn = kIOReturnSuccess;
			}
				//          ELG( rtn, foundStates, 'FndS', "privateWatchState - foundStates" );
			break;
		}
        
        // Everytime we go around the loop we have to reset the watch mask.
        // This means any event that could affect the WatchStateMask must
        // wakeup all watch state threads.  The two events are an interrupt
        // or one of the bits in the WatchStateMask changing.
        

		//IOLockLock(mWatchLock);
		
		mWatchStateMask |= mask;
        
        // note: Interrupts need to be locked out completely here,
        // since as assertwait is called other threads waiting on
        // &port->WatchStateMask will be woken up and spun through the loop.
        // If an interrupt occurs at this point then the current thread
        // will end up waiting with a different port state than assumed
        //  -- this problem was causing dequeueData to wait for a change in
        // PD_E_RXQ_EMPTY to 0 after an interrupt had already changed it to 0.
        
		assert_wait( &mWatchStateMask, true ); /* assert event */
		
		//IOLockUnlock(mWatchLock);  
			
		IORecursiveLockUnlock( mRequestLock );
		
		rtn = thread_block( (void(*)(void*, wait_result_t))0 );         /* block ourselves */

		IORecursiveLockLock( mRequestLock );

		/*if (rtn == -1 || !IsAcquired())
			return kIOReturnIPCError;
			
		IORecursiveLockLock( mRequestLock );*/

		if ( rtn == THREAD_RESTART )
		{
			continue;
		} else {
			rtn = kIOReturnIPCError;
			break;
		}
    }/* end for */

	    // As it is impossible to undo the masking used by this
	    // thread, we clear down the watch state mask and wakeup
	    // every sleeping thread to reinitialize the mask before exiting.
	
    //IOLockLock(mWatchLock);
	
	mWatchStateMask = 0;
	
	//IOLockUnlock(mWatchLock);	
	
    DLOG( "thread_wakeup_with_result waking up!\n" );
	//IORecursiveLockUnlock( mRequestLock);
	thread_wakeup_with_result( &mWatchStateMask, IsAcquired()?THREAD_RESTART:-1 );
    IORecursiveLockUnlock( mRequestLock);
        
    return rtn;
    
}

IOReturn TIUMPPort::EnqueueData(UInt8 *buffer, UInt32 size, UInt32 *count, bool sleep)
{
    UInt32   state = PD_S_TXQ_LOW_WATER;
    IOReturn rtn   = kIOReturnSuccess;
    
    DLOG("++>In Enqueue %d %d sleep-%d\n", size, *count, sleep);
    if (count == NULL || buffer == NULL)
        return kIOReturnBadArgument;
    
    (*count) = 0;
    
    if (!(ReadPortState() & PD_S_ACTIVE)) {
        return kIOReturnNotOpen;
    }
	
	/*rtn = WatchState(&state, PD_S_TX_BUSY);
	if (rtn != kIOReturnSuccess) {
		DLOG("Interrupted");
		return rtn;
	}*/

	/* OK, go ahead and try to add something to the buffer.
        */
    *count = AddtoQueue(&mTX, buffer, size);
    CheckQueues();
	
	
    /* Let the tranmitter know that we have something ready to go.
        */
    SetUpTransmit();
	
	//let's wait till all data are really sent
	/*state = 0;
	rtn = WatchState(&state, PD_S_TX_BUSY);
	if (rtn != kIOReturnSuccess) {
		DLOG("Interrupted");
		return rtn;
	}*/
    
    /* If we could not queue up all of the data on the first pass
        and the user wants us to sleep until until it's all out then sleep.
        */
    while ((*count < size) && sleep) {	
        state = PD_S_TXQ_LOW_WATER;
        rtn = WatchState(&state, PD_S_TXQ_LOW_WATER);
        if (rtn != kIOReturnSuccess) {
            DLOG("Interrupted\n");
            return rtn;
        }
        DLOG("Adding More to Queue %d\n",  size - *count);
        
        *count += AddtoQueue(&mTX, buffer + *count, size - *count);
        CheckQueues();
        
        /* Let the tranmitter know that we have something ready to go.
            */
        SetUpTransmit();
		
        state = 0;
        rtn = WatchState(&state, PD_S_TX_BUSY);
        if (rtn != kIOReturnSuccess) {
            DLOG("Interrupted\n");
            return rtn;
        }
    }
    DLOG("Enqueue Check %x\n",(int)UsedSpaceinQueue(&mTX));
    
    DLOG("chars sent = %d of %d\n", *count,size);
    DLOG("-->Out Enqueue\n");
    
    return kIOReturnSuccess;
}        

IOReturn TIUMPPort::DequeueData(UInt8 *buffer, UInt32 size, UInt32 *count, UInt32 min)
{
    IOReturn    rtn = kIOReturnSuccess;
    UInt32 	state = 0;
    
    /* Set up interrupt variable.*/
    DLOG("++>In Dequeue buf %p bufSize %d minCount %d\n",buffer, size, min);
    
    /* Check to make sure we have good arguments.
        */
    if ((count == NULL) || (buffer == NULL) || (min > size))
        return kIOReturnBadArgument;
    
    /* If the port is not active then there should not be any chars.
        */
    *count = 0;
    if (!(ReadPortState() & PD_S_ACTIVE))
        return(kIOReturnNotOpen);
    
        
    /* Get any data living in the queue.
        */
    *count = RemovefromQueue(&mRX, buffer, size);
    CheckQueues();
    DLOG("-->chars receivedA = %d of needed %d, min = %d\n", *count, size, min);
    
#if 1
    while ((min > 0) && (*count < min)) {
        /* Figure out how many bytes we have left to queue up */
        state = 0;
        
        rtn = WatchState(&state, PD_S_RXQ_EMPTY);
        
        // Test This                if ((rtn == IO_R_IO) || (rtn == kIOReturnSuccess)) {
        if (rtn == kIOReturnSuccess) {
            //this may cause the queues to get corrupted so leaving ints enabled for now
            //IOSimpleLockUnlockEnableInterrupt(port->serialRequestLock, previousInterruptState);
        } else {
            DLOG("Interrupted! - %08X\n",rtn);
            return rtn;
        }
        /* Try and get more data starting from where we left off */
        *count += RemovefromQueue(&mRX, buffer + *count, (size - *count));
        CheckQueues();
        DLOG("-->chars receivedB = %d of needed %d\n", *count, size);
        }
#endif
        
    return rtn;
}

void TIUMPPort::FreeRingBuffer(CirQueue *Queue)
{
    if (Queue->Start) {
        IOFree(Queue->Start, Queue->Size);
        CloseQueue(Queue);
    }
}

bool TIUMPPort::AllocateRingBuffer(CirQueue *Queue, size_t BufferSize)
{
    u_char		*Buffer;
    
    Buffer = (u_char *)IOMalloc(BufferSize);
    
    InitQueue(Queue, Buffer, BufferSize);
    
    if (Buffer)
        return true;
    else
        return false;
}

void TIUMPPort::SetHWStatus(tUmpPortStatus hwState)
{
    //IORecursiveLockLock( mRequestLock);
	
	mHWPortStatus = hwState;
	
	if ( true/*IORecursiveLockTryLock( mRequestLock )*/ )
	{
		//IORecursiveLockUnlock( mRequestLock);

		UpdateStatus();
	
		DLOG("SetHWStatus - lock succeeded\n");
	}
	else
	{
		DLOG("SetHWStatus - lock failed\n");
	}
	
    //IORecursiveLockUnlock( mRequestLock);
}

void TIUMPPort::GetHWStatus(tUmpPortStatus &hwState)
{
    //IORecursiveLockLock( mRequestLock);
	
	hwState = mHWPortStatus;
		
    //IORecursiveLockUnlock( mRequestLock);
}


void TIUMPPort::UpdateStatus(void)
{
	DLOG( "UpdateStatus - begin\n" );
	
	if (ReadPortState() & (PD_S_ACTIVE | PD_S_ACQUIRED)) {
		UInt32 state = ReadPortState();
		
		if (mHWPortStatus.bPortStatus1 & (1 << 4)) { //bit 4 - CTS
			state |= PD_RS232_S_CTS;
		} else {
			state &= ~PD_RS232_S_CTS;
		}
		
		if (mHWPortStatus.bPortStatus1 & (1 << 5)) { //bit 5 - DSR
			state |= PD_RS232_S_DSR;
		} else {
			state &= ~PD_RS232_S_DSR;
		}

		if (mHWPortStatus.bPortStatus1 & (1 << 6)) { //bit 6 - RING
			state |= PD_RS232_S_RNG;
		} else {
			state &= ~PD_RS232_S_RNG;
		}
		
		if (mHWPortStatus.bPortStatus1 & (1 << 7)) { //bit 7 - DCD
			state |= PD_RS232_S_CAR;
		} else {
			state &= ~PD_RS232_S_CAR;
		}
		
		if (mHWPortStatus.bPortStatus2 & (1 << 3)) { //bit 3 - Break interrupt
			state |= PD_RS232_S_BRK;
		} else {
			state &= ~PD_RS232_S_BRK;
		}
		
		mOSPortConfig.OverrunError = ((mHWPortStatus.bPortStatus2 & (1 << 0))>0);
		mOSPortConfig.ParityError = ((mHWPortStatus.bPortStatus2 & (1 << 1))>0);
		mOSPortConfig.FramingError = ((mHWPortStatus.bPortStatus2 & (1 << 2))>0);
		mOSPortConfig.RXFull = ((mHWPortStatus.bPortStatus2 & (1 << 4))>0);
		mOSPortConfig.TXEmpty = ((mHWPortStatus.bPortStatus2 & (1 << 5))>0);

		/* Figure out what has changed to get mask.*/
		UInt32 deltastate = state ^ ReadPortState();
		ChangeState( state, deltastate );
	}
}

IOReturn TIUMPPort::ReloadStatus(void)
{   
    IORecursiveLockLock( mRequestLock);

	DLOG("Reload Status - start\n");	

	mUMPDevice->GetPortStatus(this,&mHWPortStatus);
	
    DLOG("ReloadStatus - %02X - %02X\n",mHWPortStatus.bPortStatus1, mHWPortStatus.bPortStatus2);
	
	UpdateStatus();
	
	IORecursiveLockUnlock( mRequestLock);

    
    return kIOReturnSuccess;
}

IOReturn TIUMPPort::ReloadConfig(void)
{       
    IORecursiveLockLock( mRequestLock);

    mUMPDevice->GetPortConfig(this,&mHWPortConfig);
            
    IORecursiveLockUnlock( mRequestLock);

    return kIOReturnSuccess;
}

IOReturn TIUMPPort::SetUpTransmit( void )
{
    size_t      count = 0;
    size_t      data_Length;
        
  	//  If we are already in the cycle of transmitting characters,
    //  then we do not need to do anything.
    
    if ( mAreTransmitting || mWriteActive)
		return false;
		
    
    // First check if we can actually do anything, also if IrDA has no room we're done for now
    
    //if ( GetQueueStatus( &fPort->TX ) != queueEmpty )
    if ((data_Length = UsedSpaceinQueue(&mTX)) > 0)
    {
        if (data_Length > MAX_BLOCK_SIZE)
			data_Length = MAX_BLOCK_SIZE;
        
        mPipeOutMDP->setLength( data_Length );

		count = RemovefromQueue( &mTX, mPipeOutBuffer, data_Length );
        
		mAreTransmitting = true;
		ChangeState( PD_S_TX_BUSY, PD_S_TX_BUSY );
        
		mWriteActive=true;
	
		DLOG("num data %ld\n",data_Length);
		
		DEVLogData(kUSBOut,data_Length,(char*)mPipeOutBuffer);
        
		//mPipeOut->Write( mPipeOutMDP, 1000, 1000, &mWriteCompletionInfo );
		mPipeOut->Write( mPipeOutMDP, &mWriteCompletionInfo );
		
		mAreTransmitting = false;
    }
    
    return kIOReturnSuccess;
}/* end SetUpTransmit */



void TIUMPPort::SetOSConfig(TIUMPPort::OSConfig &cfg, UInt32 mask)
{
	bool hwwrite = false;
	if (mask & kMskBaudrate) {
		mOSPortConfig.BaudRate = cfg.BaudRate;
		WORD bd = mUMPDevice->EncodeBaudRate(cfg.BaudRate);
		if (bd != mHWPortConfig.wBaudRate) {
			hwwrite = true;
			mHWPortConfig.wBaudRate=bd;
		}
	}
	
	if (mask & kMskCharBits) {
		mOSPortConfig.CharBits = cfg.CharBits;
		BYTE cb;
		switch (cfg.CharBits) {
			case 5:
				cb=UMP_UART_CHAR5BITS;
				break;
			case 6:
				cb=UMP_UART_CHAR6BITS;
				break;
			case 7:
				cb=UMP_UART_CHAR7BITS;
				break;
			case 8:
				cb=UMP_UART_CHAR8BITS;
				break;
		}
		if (cb != mHWPortConfig.bDataBits) {
			hwwrite = true;
			mHWPortConfig.bDataBits=cb;
		}
	}
	
	if (mask & kMskParity) {
		mOSPortConfig.Parity = cfg.Parity;
		BYTE pr;
		switch (cfg.Parity) {
			case PD_RS232_PARITY_NONE:
				pr=UMP_UART_NOPARITY;
				break;
			case PD_RS232_PARITY_ODD:
				pr=UMP_UART_ODDPARITY;
				break;
			case PD_RS232_PARITY_EVEN:
				pr=UMP_UART_EVENPARITY;
				break;
			case PD_RS232_PARITY_MARK:
				pr=UMP_UART_MARKPARITY;
				break;
			case PD_RS232_PARITY_SPACE:
				pr=UMP_UART_SPACEPARITY;
				break;
		}
		if (pr != mHWPortConfig.bParity) {
			hwwrite = true;
			mHWPortConfig.bParity=pr;
		}
	}
	
	if (mask & kMskStopBits) {
		mOSPortConfig.StopBits = cfg.StopBits;
		BYTE sb;
		switch (cfg.StopBits) {
			case 2:
				sb=UMP_UART_STOPBIT1;
				break;
			case 3:
				sb=UMP_UART_STOPBIT15;
				break;
			case 4:
				sb=UMP_UART_STOPBIT2;
				break;
		}
		if (sb != mHWPortConfig.bStopBits) {
			hwwrite = true;
			mHWPortConfig.bStopBits=sb;
		}
	}
	
	if (mask & kMskXONChar) {
		mOSPortConfig.XONChar = cfg.XONChar;
		if (mOSPortConfig.XONChar != mHWPortConfig.cXon) {
			hwwrite = true;
			mHWPortConfig.cXon=mOSPortConfig.XONChar;
		}
	}

	if (mask & kMskXOFFChar) {
		mOSPortConfig.XOFFChar = cfg.XOFFChar;
		if (mOSPortConfig.XOFFChar != mHWPortConfig.cXoff) {
			hwwrite = true;
			mHWPortConfig.cXoff=mOSPortConfig.XOFFChar;
		}
	}
	
	if (mask & kMskFlowControl) {
		WORD wrd = mHWPortConfig.wFlags & (UMP_MASK_UART_FLAGS_DTR_DISABLE | UMP_MASK_UART_FLAGS_RTS_DISABLE);
		mOSPortConfig.FlowControl = cfg.FlowControl;

		if (cfg.FlowControl&PD_RS232_A_DTR) {
			wrd |= UMP_MASK_UART_FLAGS_DTR_FLOW;
			wrd &= ~UMP_MASK_UART_FLAGS_DTR_DISABLE;
		}
		
		if (cfg.FlowControl&PD_RS232_A_RTS) {
			wrd |= UMP_MASK_UART_FLAGS_RTS_FLOW;
			wrd &= ~UMP_MASK_UART_FLAGS_RTS_DISABLE;
		}

		if (cfg.FlowControl&PD_RS232_A_TXO) {
			wrd |= UMP_MASK_UART_FLAGS_OUT_X;
		}

		if (cfg.FlowControl&PD_RS232_A_RXO) {
			wrd |= UMP_MASK_UART_FLAGS_IN_X;
		}
		
		if (cfg.FlowControl&PD_RS232_A_CTS) {
			wrd |= UMP_MASK_UART_FLAGS_OUT_X_CTS_FLOW;
		}

		if (cfg.FlowControl&PD_RS232_A_DSR) {
			wrd |= UMP_MASK_UART_FLAGS_OUT_X_DSR_FLOW;
		}

		if (cfg.FlowControl&PD_RS232_A_XANY) {
			wrd |= UMP_MASK_UART_FLAGS_OUT_XA;
		}

		if (wrd != mHWPortConfig.wFlags) {
			hwwrite = true;
			mHWPortConfig.wFlags=wrd;
		}
	}
	
	if (mask & kMskState) {
		WORD wrd = mHWPortConfig.wFlags;
		if (!(mOSPortConfig.FlowControl & PD_RS232_A_DTR)) {
			if (mOSPortState & PD_RS232_S_DTR)
				wrd &= ~UMP_MASK_UART_FLAGS_DTR_DISABLE;
			else
				wrd |= UMP_MASK_UART_FLAGS_DTR_DISABLE;
			
		}
		
		if (!(mOSPortConfig.FlowControl & PD_RS232_A_RTS)) {
			if (mOSPortState & PD_RS232_S_RTS)
				wrd &= ~UMP_MASK_UART_FLAGS_RTS_DISABLE;
			else
				wrd |= UMP_MASK_UART_FLAGS_RTS_DISABLE;
			
		}
		
		if (wrd != mHWPortConfig.wFlags) {
			hwwrite = true;
			mHWPortConfig.wFlags=wrd;
		}
	}

	if (hwwrite) {
		if (mHWPortConfig.bParity != UMP_UART_NOPARITY)
			mHWPortConfig.wFlags |= UMP_MASK_UART_FLAGS_PARITY;
		else
			mHWPortConfig.wFlags &= ~UMP_MASK_UART_FLAGS_PARITY;
		mUMPDevice->SetPortConfig(this,&mHWPortConfig);
	}
}

void TIUMPPort::GetOSConfig(TIUMPPort::OSConfig &cfg)
{
	cfg=mOSPortConfig;
}

void TIUMPPort::SetOSConfigDefaults(void)
{
	mOSPortConfig.BaudRate = 9600;
	mOSPortConfig.CharBits = 8;
	mOSPortConfig.StopBits = 2;
	mOSPortConfig.Parity = 0;
	mOSPortConfig.XONChar = 0x11;
	mOSPortConfig.XOFFChar = 0x13;
	mOSPortConfig.FlowControl = (DEFAULT_AUTO | DEFAULT_NOTIFY);
	mOSPortConfig.OverrunError = false;
	mOSPortConfig.ParityError = false;
	mOSPortConfig.FramingError = false;
	mOSPortConfig.RXFull = false;
	mOSPortConfig.TXEmpty = false;
}


IOReturn TIUMPPort::RequestEvent(UInt32 event, UInt32 *data)
{
    IOReturn returnValue = kIOReturnSuccess;
	
	TIUMPPort::OSConfig cfg;
	
	GetOSConfig(cfg);
	
	switch (event) {
		case PD_E_ACTIVE				: 	*data = bool(ReadPortState()&PD_S_ACTIVE);		break;
		case PD_E_FLOW_CONTROL			:	*data = cfg.FlowControl;							break;
		case PD_E_DELAY					:	*data = 500/1000;		break;
		case PD_E_DATA_LATENCY			:	*data = 500/1000;		break;
		case PD_E_TXQ_SIZE				:	*data = GetQueueSize(&mTX);			break;
		case PD_E_RXQ_SIZE				:	*data = GetQueueSize(&mRX);			break;
		case PD_E_TXQ_LOW_WATER			:	*data = mTXStats.LowWater;							break;
		case PD_E_RXQ_LOW_WATER			:	*data = mRXStats.LowWater;							break;
		case PD_E_TXQ_HIGH_WATER		:	*data = mTXStats.HighWater;							break;
		case PD_E_RXQ_HIGH_WATER		:	*data = mRXStats.HighWater;							break;
		case PD_E_TXQ_AVAILABLE			:	*data = FreeSpaceinQueue(&mTX);				break;
		case PD_E_RXQ_AVAILABLE			:	*data = UsedSpaceinQueue(&mRX);				break;
		case PD_E_DATA_RATE				:	*data = cfg.BaudRate << 1;						break;
		case PD_E_RX_DATA_RATE			:	*data = 0x00;										break;
		case PD_E_DATA_SIZE				: 	*data = cfg.CharBits << 1; 						break;
		case PD_E_RX_DATA_SIZE			:	*data = 0x00;										break;
		case PD_E_DATA_INTEGRITY		:	*data = cfg.Parity;							break;
		case PD_E_RX_DATA_INTEGRITY		:	*data = cfg.Parity;							break;
		//stopbits are internaly stored in halfbits, so don't double it here.
		case PD_RS232_E_STOP_BITS		:	*data = cfg.StopBits;						break;
		case PD_RS232_E_RX_STOP_BITS	:	*data = 0x00;										break;
		case PD_RS232_E_XON_BYTE		:	*data = cfg.XONChar;								break;
		case PD_RS232_E_XOFF_BYTE		:	*data = cfg.XOFFChar;								break;
		case PD_RS232_E_LINE_BREAK		:	*data = bool(ReadPortState()&PD_RS232_S_BRK);	break;
		case PD_RS232_E_MIN_LATENCY		:	*data = bool(false);						break;
		default 						:	returnValue = kIOReturnBadArgument; 				break;
	}

    return returnValue;
}

IOReturn TIUMPPort::ExecuteEvent(UInt32 event, UInt32 data)
{    
    IOReturn ret = kIOReturnSuccess;

	//DLOG("ExecuteEvent-1 %d %d\n",(int)(event&PD_E_MASK)>>2,(int)data);

    UInt32 delta = 0;
	UInt32 state = ReadPortState();
	
	if ((state & PD_S_ACQUIRED) == 0)
        return kIOReturnNotOpen;
		
	TIUMPPort::OSConfig cfg;

	//DLOG("ExecuteEvent-2 %d %d\n",(int)(event&PD_E_MASK)>>2,(int)data);

	
	switch (event) {
      case PD_RS232_E_XON_BYTE:
			cfg.XONChar = data;
			SetOSConfig(cfg,kMskXONChar);
            break;

        case PD_RS232_E_XOFF_BYTE:
			cfg.XOFFChar = data;
			SetOSConfig(cfg,kMskXOFFChar);
            break;
	/*
        case PD_E_SPECIAL_BYTE:
            port->SWspecial[data>>SPECIAL_SHIFT] |= (1 << (data & SPECIAL_MASK));
            break;

        case PD_E_VALID_DATA_BYTE:
            port->SWspecial[data>>SPECIAL_SHIFT] &= ~(1 << (data & SPECIAL_MASK));
            break;
*/

// Enqueued flow control event allows the user to change the state of any flow control
// signals set on Manual (its Auto bit in exec/req event is cleared)
        case PD_E_FLOW_CONTROL :
DLOG("-ExecuteEvent PD_E_FLOW_CONTROL - %08X\n",data);
			cfg.FlowControl = data;
			SetOSConfig(cfg,kMskFlowControl);
            /*tmp = data & (PD_RS232_D_RTS | PD_RS232_D_DTR);
            tmp >>= PD_RS232_D_SHIFT;
            tmp &= ~(port->FlowControl);
            *delta |= tmp;
            *state &= ~tmp;
            *state |= (tmp & data);
            if (tmp & PD_RS232_S_RXO) {
                if (data & PD_RS232_S_RXO)
				{
                    port->RXOstate = NEEDS_XON;
				}
                else
				{
                    port->RXOstate = NEEDS_XOFF;
				}
            }
#if 0
            programMCR(port, *state);
#endif*/
            break;

        case PD_E_ACTIVE:
DLOG("-ExecuteEvent PD_E_ACTIVE\n");
            if ((bool)data)
                ret = Start();
            else
                ret = Stop();
            break;

         /* //Fix for IRDA and MIDI which want to know about bytes as soon as it arrives      
        case PD_E_DATA_LATENCY:
                port->DataLatInterval = long2tval(data *1000);
               // IOLog("Setting PD_E_DATA_LATENCY to: %ld \n",data);
            break;

        case PD_RS232_E_MIN_LATENCY:
            port->MinLatency = bool(data);
            port->DLRimage = 0x0000;
            programChip(port);
            break;
*/
		case PD_E_DATA_INTEGRITY:
DLOG("-ExecuteEvent PD_E_DATA_INTEGRITY %d\n",(int)data);
            if ((data < PD_RS232_PARITY_NONE) || (data > PD_RS232_PARITY_SPACE)) {
                ret = kIOReturnBadArgument;
            }
            else {
				cfg.Parity = data;
				SetOSConfig(cfg,kMskParity);
            }
            break;

		 case PD_E_DATA_RATE:
           // For API compatiblilty with Intel.
            data >>=1;
			DLOG("-ExecuteEvent PD_E_DATA_RATE %d\n",(int)data);
            if ((data < 300) || (data > kMaxBaudRate))
                ret = kIOReturnBadArgument;
            else {
				cfg.BaudRate = data;
				SetOSConfig(cfg,kMskBaudrate);
            }
            break;
/*
		case PD_E_EXTERNAL_CLOCK_MODE:
			//  For compatiblilty with MIDI.
DLOG("-ExecuteEvent PD_E_EXTERNAL_CLOCK_MODE data = %d\n", data);
			switch (data) {
				case kX1UserClockMode:				// PPCSerialPort.h for value
					clockMode = kX1ClockMode;		// Z85C30.h for value
					break;
				case kX16UserClockMode:
					clockMode = kX16ClockMode;
					break;
				case kX32UserClockMode:
					clockMode = kX32ClockMode;
					break;
				case kX64UserClockMode:
					clockMode = kX64ClockMode;
					break;
				default :
					clockMode = kX32ClockMode;
					break;
			}	
			if (!SccConfigureForMIDI(port, clockMode))
				ret = kIOReturnBadArgument;
			break;
*/
        case PD_E_DATA_SIZE:
            // For API compatiblilty with Intel.
            data >>=1;
            DLOG("-ExecuteEvent PD_E_DATA_SIZE data = %d\n", data);
            if ((data < 5) || (data > 8))
                ret = kIOReturnBadArgument;
            else {
				cfg.CharBits = data;
				SetOSConfig(cfg,kMskCharBits);
            }
            break;
        case PD_RS232_E_STOP_BITS:
            DLOG("-ExecuteEvent PD_RS232_E_STOP_BITS\n");
            if ((data < 0) || (data > 20))
                ret = kIOReturnBadArgument;
            else {
				cfg.StopBits = data;
				SetOSConfig(cfg,kMskStopBits);
            }
            break;
/*
        case PD_E_RX_DATA_INTEGRITY:
            if ((data != PD_RS232_PARITY_DEFAULT) && (data != PD_RS232_PARITY_ANY))
                ret = kIOReturnBadArgument;
            else {
				cfg.Parity = data;
				SetOSConfig(cfg,kMskParity);
            }
            break;
        case PD_E_RX_DATA_RATE:
            if (data != 0)
                ret = kIOReturnBadArgument;
            break;
        case PD_E_RX_DATA_SIZE:
            if (data != 0)
                ret = kIOReturnBadArgument;
            break;
        case PD_RS232_E_RX_STOP_BITS:
            if (data != 0)
                ret = kIOReturnBadArgument;
            break;
*/
        case PD_RS232_E_LINE_BREAK :
            state &= ~PD_RS232_S_BRK;
            delta |= PD_RS232_S_BRK;
            break;
  
/*
		case PD_E_DELAY:
            assert(!((*state) & INTERNAL_DELAY));
            if (data > 0) {
                if (data > (UINT_MAX/1000))
                    data = (UINT_MAX/1000);
                *state |= INTERNAL_DELAY;
                //calloutEntryDispatchDelayed(port->DelayTOEntry, calloutDeadlineFromInterval(long2tval(data*1000)));
            }
            break;*/
                
          default:
                ret = kIOReturnSuccess;
                break;
        }
		
		state |= state;/* ejk for compiler warnings. ?? */
		ChangeState( state, delta );
		
		return ret;
}







