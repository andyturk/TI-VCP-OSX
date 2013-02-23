/*
 *TIUMPSerial.cpp
 *
 *MacOSX implementation of USB-Serial Converter
 */

#include <machine/limits.h>			/* UINT_MAX */
#include <IOKit/assert.h>

#include <sys/kdebug.h>

#include <libkern/OSByteOrder.h>

#include <IOKit/IOLib.h>
#include <IOKit/IOReturn.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/serial/IORS232SerialStreamSync.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/usb/IOUSBDevice.h>
#include <IOKit/IOMessage.h>

#include <IOKit/IORegistryEntry.h>


#include "UMPSerialDriver.h"
#include "TIUMPPrimitives.h"

#define IOSS_HALFBIT_BRD 1 
//#define USE_TIMER_EVENT_SOURCE_DEBUGGING 1
//#define kTimerTimeout 3000

enum {
    kPowerOffState  = 0,
    kPowerOnState   = 1,
    kNumPowerStates = 2
};


static IOPMPowerState gOurPowerStates[kNumPowerStates] = {
    {1,0,0,0,0,0,0,0,0,0,0,0},
    {1,IOPMDeviceUsable,IOPMPowerOn,IOPMPowerOn,0,0,0,0,0,0,0,0}
};

// This is to check the status of the debug flags
#include <pexpert/pexpert.h>

extern IORecursiveLock* gDebugLock;

OSDefineMetaClassAndStructors(TIUMPSerial, IOSerialDriverSync)

#define super IOSerialDriverSync


IOService*  TIUMPSerial::probe( IOService *provider, SInt32 *pScore )
{
    fProvider = OSDynamicCast(IOUSBDevice, provider);
    if (!fProvider)
        return false;
    
    fProvider->open(this);

    TIUMPDevice::ChipIDT chipid = (TIUMPDevice::ChipIDT)-1;
    
    OSString *chipsetstr = OSDynamicCast(OSString, getProperty("TIUMPChipset"));
    if ((chipsetstr != NULL) && (chipsetstr->isEqualTo("3410")))
        chipid = TIUMPDevice::kChip3410;
    
    if ((chipsetstr != NULL) && (chipsetstr->isEqualTo("5052")))
        chipid = TIUMPDevice::kChip5052;
    
    if (chipid == -1)
        return false;
    
    DLOG("Chipset: %d\n", chipid);
    
    //this constructor upload firmware if necessary.
    mUMPDevice = new TIUMPDevice(this, fProvider, chipid);
    
    if (!mUMPDevice->IsFWLoaded()) {
        mUMPDevice->UploadFirmware();
        fProvider->close(this);
        return NULL;
    }
        
    fProvider->close(this);
        
    return this;
}

/*
 *TIUMPSerial::start
 *note returning false currently causes a crash
 */
bool TIUMPSerial::start(IOService *provider)
{        
    if (!super::start(provider))
        return false;
    
	fTerminated = true;
	
    IOUSBDevice *newProvider = OSDynamicCast(IOUSBDevice, provider);
    
    if (mUMPDevice==NULL || newProvider!=fProvider) {
        fProvider = newProvider;
        if (!fProvider)
            return false;
        
        fProvider->open(this);
        
        TIUMPDevice::ChipIDT chipid = (TIUMPDevice::ChipIDT)-1;
        
        OSString *chipsetstr = OSDynamicCast(OSString, getProperty("TIUMPChipset"));
	if ((chipsetstr != NULL) && (chipsetstr->isEqualTo("3410")))
            chipid = TIUMPDevice::kChip3410;
        
	if ((chipsetstr != NULL) && (chipsetstr->isEqualTo("5052")))
            chipid = TIUMPDevice::kChip5052;
        
        if (chipid == -1)
            return false;
        
        mUMPDevice = new TIUMPDevice(this, fProvider, chipid);
    } else {
        fProvider->open(this);
    }
    
	//fProvider->retain();
	
    fTerminate = false;
            
    if (mUMPDevice->EnumeratePorts()!=DFW_ERR_NO_ERROR) {
        fProvider->close(this);
        delete mUMPDevice;
        return false;
    }
    
    DLOG("TIUMPSerial: All initialization done.\n");
    
	fTerminated = false;
	
	return initForPM(provider);
}

bool TIUMPSerial::initForPM(IOService * provider)
{    
    fPowerState = kPowerOnState;        // init our power state to be 'on'
    PMinit();                               // init power manager instance variables
    provider->joinPMtree(this);             // add us to the power management tree

    // register ourselves with ourself as policy-maker
    registerPowerDriver(this, gOurPowerStates, kNumPowerStates);
    return true;
}

//
// request for our initial power state
//
unsigned long TIUMPSerial::initialPowerStateForDomainState ( IOPMPowerFlags flags)
{
    return fPowerState;
}

//
// request to turn device on or off
//
IOReturn TIUMPSerial::setPowerState(unsigned long powerStateOrdinal, IOService * whatDevice)
{    
    if (powerStateOrdinal != kPowerOffState && powerStateOrdinal != kPowerOnState)
		return IOPMNoSuchState;

    if (powerStateOrdinal == fPowerState)
		return IOPMAckImplied;

    fPowerState = powerStateOrdinal;
    
    return IOPMNoErr;
}


void TIUMPSerial::stop(IOService *provider)
{
    DLOG("stop\n");
    
    if (fTerminated)
		return;

    DoTerminate();
		    
   super::stop(provider);
}

bool TIUMPSerial::willTerminate(IOService * provider, IOOptionBits options)
{		    
	DLOG("willTerminate\n");
	
	if (mUMPDevice)
		mUMPDevice->SetTerminate(fTerminate);
				
   mUMPDevice->Terminate();
   
   return super::willTerminate(provider, options);
}


bool TIUMPSerial::didTerminate( IOService * provider, IOOptionBits options, bool * defer )

{		    
	DLOG("didTerminate\n");
	   
   fProvider->close(this);
      
   return super::didTerminate(provider, options, defer);
}


void TIUMPSerial::DoTerminate(void)
{
	//When disconnecting from USB, this function can be called for every port. But we need to call it just once.
    if (fTerminated)
		return;
	
	DLOG("DoTerminate\n");
    if (mUMPDevice) {
        delete mUMPDevice;
        mUMPDevice = NULL;
    }
    
	fProvider->stop(this);

	PMstop();
	
	fTerminated = true;
	
	//fProvider->release();
}

IOReturn TIUMPSerial::message( UInt32 type, IOService *provider,  void *argument)
{
    switch ( type )
    {
	//case kIOMessageServiceIsTerminated:
	
	//		DLOG("kIOMessageServiceIsTerminated\n");
    //        fTerminate = true;  
     //       break;
	    
	case kIOUSBMessagePortHasBeenResumed: 
            DLOG("kIOUSBMessagePortHasBeenResumed\n");
            fTerminate = false;  
            break;
            
	case kIOUSBMessageHubResumePort:
            DLOG("kIOUSBMessageHubResumePort\n");
            fTerminate = true;  
            break;
            
        default:
            break;
    }
	
	if (fTerminate)
	{
		if (mUMPDevice)
			mUMPDevice->SetTerminate(fTerminate);
	}
    
    return kIOReturnSuccess;
}

// =================================================
// New implementation of PortDevice protocol
// =================================================
/* acquirePort tests and sets the state of the port object.  If the port was
 *available, then the state is set to busy, and kIOReturnSuccess is returned.
 *If the port was already busy and sleep is YES, then the thread will sleep
 *until the port is freed, then re-attempts the acquire.  If the port was
 *already busy and sleep is NO, then IO_R_EXCLUSIVE_ACCESS is returned.
 */
IOReturn TIUMPSerial::acquirePort(bool sleep, void *refCon)
{
    DLOG("acquirePort\n");
			
    TIUMPPort *port = (TIUMPPort *)refCon;
		    
    if (port!=NULL) {
        for (;;) {
            UInt32 busyState = port->ReadPortState() & PD_S_ACQUIRED;
            if (!busyState) {
				return port->Acquire();
                break;
            } else {
                if (!sleep) {
                     return kIOReturnExclusiveAccess;
                } else {
                    busyState = 0;
                    IOReturn rtn = watchState( &busyState, PD_S_ACQUIRED, refCon );
                    if ( (rtn == kIOReturnIOError) || (rtn == kIOReturnSuccess) )
                    {
                        continue;
                    } else {
                        DLOG("acquirePort - Interrupted!");
                        return rtn;
                    }
                }
            }
        }
        
    }
    
    return kIOReturnBadArgument;
}

/* release sets the state of the port object to available and wakes up and
 *threads sleeping for access to this port.  It will return IO_R_SUCCESS
 *if the port was in a busy state, and IO_R_NOT_OPEN if it was available.
 */
IOReturn TIUMPSerial::releasePort(void *refCon)
{
    IOReturn rtn = kIOReturnSuccess;
    UInt32 busyState;
    
    DLOG("releasePort\n");
	
	//if we already terminated ourselves, don't do it again.
	if (fTerminated)
		return kIOReturnSuccess;
		    
    TIUMPPort *port = (TIUMPPort *)refCon;
    
    if (port != NULL) {
        busyState = (port->ReadPortState() & PD_S_ACQUIRED);
        if ( !busyState )
        {
            rtn=kIOReturnNotOpen;
			goto End;
        }
        
        //port->ChangeState(0, (UInt32)STATE_ALL);  // Clear the entire state word which also deactivates the port
        
        rtn = port->Release();
		
		DLOG ("port->Release() returned;");
    } else {
        rtn=kIOReturnBadArgument;
		goto End;
    }

End:
    DLOG("port released\n");

    if (fTerminate)
        stop(fProvider);//DoTerminate();
	
    return rtn;
}

/*
 *Set the state for the port device.  The lower 16 bits are used to set the
 *state of various flow control bits (this can also be done by enqueueing an
 *PD_E_FLOW_CONTROL event).  If any of the flow control bits have been set
 *for automatic control, then they can't be changed by setState.  For flow
 *control bits set to manual (that are implemented in hardware), the lines
 *will be changed before this method returns.  The one weird case is if RXO
 *is set for manual, then an XON or XOFF character may be placed at the end
 *of the TXQ and transmitted later.
 */
IOReturn TIUMPSerial::setState(UInt32 state, UInt32 mask, void *refCon)
{
    TIUMPPort *port = (TIUMPPort *) refCon;
	//IOPanic("setState");
			
	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;

    if (mask & (PD_S_ACQUIRED | PD_S_ACTIVE | (~EXTERNAL_MASK)))
        return kIOReturnBadArgument;

    if (port != NULL) {
        if ((port->ReadPortState() & PD_S_ACQUIRED) != 0) {
			TIUMPPort::OSConfig cfg;
	
			port->GetOSConfig(cfg);

            // ignore any bits that are read-only
            mask &= (~cfg.FlowControl & PD_RS232_A_MASK) | PD_S_MASK;
			
			DLOG("++>setState %08X %08X\n",(int)state, (int)mask);

            if (mask != 0)
                port->ChangeState(state, mask);

            DLOG("-->setState\n");
            return kIOReturnSuccess;
        }
    } else {
        return kIOReturnBadArgument;
    }

    return kIOReturnNotOpen;
}

/*
 *Get the state for the port device.
 */
UInt32 TIUMPSerial::getState(void *refCon)
{
    DLOG("getState\n");
		    
	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;

    TIUMPPort *port = (TIUMPPort *) refCon;
    
    if (port != NULL /*&& (port->ReadPortState() & PD_S_ACQUIRED)*/) {
        port->CheckQueues();
       return (port->ReadPortState() & EXTERNAL_MASK);
    } else {
        return 0;
    }
}

/*
 *Wait for the at least one of the state bits defined in mask to be equal
 *to the value defined in state. Check on entry then sleep until necessary.
 */
IOReturn TIUMPSerial::watchState(UInt32 *state, UInt32 mask, void *refCon)
{
        
 	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;

   TIUMPPort *port = (TIUMPPort *) refCon;
    IOReturn ret = kIOReturnNotOpen;

    DLOG("watchState - state %08X mask %08X\n",*state, mask);
	
    if (port != NULL) {
        if ((port->ReadPortState() & PD_S_ACQUIRED) != 0) {
            ret = kIOReturnSuccess;
            mask &= EXTERNAL_MASK;
            ret = port->WatchState(state, mask);
            (*state) &= EXTERNAL_MASK;
        }
    } else {
        ret = kIOReturnBadArgument;
    }
    
    return ret;
}


/* nextEvent returns the type of the next event on the RX queue.  If no
 *events are present on the RX queue, then PD_E_EOQ is returned.
 */
UInt32 TIUMPSerial::nextEvent(void *refCon)
{    
    UInt32 	ret = kIOReturnSuccess;

	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;

    DLOG("nextEvent\n");
	
//    PortInfo_t *port = (PortInfo_t *) refCon;
//	ret=peekEvent(&(port->RX), 0);

    return ret;
}

/* executeEvent causes the specified event to be processed immediately.
 *This is primarily used for channel control commands like START & STOP
 */
IOReturn TIUMPSerial::executeEvent(UInt32 event, UInt32 data, void *refCon)
{
	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;

    TIUMPPort *port = (TIUMPPort *) refCon;
 		
	if (port!=NULL)
		return port->ExecuteEvent(event,data);
		
	return kIOReturnBadArgument;
}

/* requestEvent processes the specified event as an immediate request and
 *returns the results in data.  This is primarily used for getting link
 *status information and verifying baud rate and such.
 *
 *Author's note:  This was one of my favorite routines to code up!
 */
IOReturn TIUMPSerial::requestEvent(UInt32 event, UInt32 *data, void *refCon)
{
	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;

    TIUMPPort *port = (TIUMPPort *) refCon;
		
	if (port==NULL || data == NULL)
		return kIOReturnBadArgument;

    DLOG("call requestEvent %u\n", (UInt32)(event&PD_E_MASK)>>2);
	//DLOG("requestEvent\n");
	
	return port->RequestEvent(event,data);
}

/* enqueueEvent will place the specified event into the TX queue.  The
 *sleep argument allows the caller to specify the enqueueEvent's
 *behaviour when the TX queue is full.  If sleep is true, then this
 *method will sleep until the event is enqueued.  If sleep is false,
 *then enqueueEvent will immediatly return IO_R_RESOURCE.
 */
IOReturn TIUMPSerial::
enqueueEvent(UInt32 event, UInt32 data, bool sleep, void *refCon)
{
	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;

    TIUMPPort *port = (TIUMPPort *) refCon;
    DLOG("enqueueEvent (write) %d\n", (event&PD_E_MASK)>>2);

    if ((port->ReadPortState() & PD_S_ACTIVE) != 0) {
        //	rtn = TX_enqueueEvent(port, event, data, sleep);

#ifdef IntelTest
        if ((rtn == kIOReturnSuccess) && (!((readPortState(port))&PD_S_TX_BUSY)))
            calloutEntryDispatch(port->FrameTOEntry);	// restart TX engine
#endif
        return kIOReturnSuccess;
    }

    return kIOReturnNotOpen;
}

/* dequeueEvent will remove the oldest event from the RX queue and return
 *it in event & data.  The sleep argument defines the behavior if the RX
 *queue is empty.  If sleep is true, then this method will sleep until an
 *event is available.  If sleep is false, then an PD_E_EOQ event will be
 *returned.  In either case IO_R_SUCCESS is returned.
 */
IOReturn TIUMPSerial::
dequeueEvent(UInt32 *event, UInt32 *data, bool sleep, void *refCon)
{
	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;

    TIUMPPort *port = (TIUMPPort *) refCon;
    DLOG("dequeueEvent (read)\n");
	
    if ((event == NULL) || (data == NULL) || (port == NULL))
        return kIOReturnBadArgument;

    if ((port->ReadPortState() & PD_S_ACTIVE) !=0) {
        //	rtn = RX_dequeueEvent(port, &e, data, sleep);
        //	*event = (UInt32)e;
        return kIOReturnSuccess;
    }

    return kIOReturnNotOpen;
}

/* enqueueData will attempt to copy data from the specified buffer to the
 *TX queue as a sequence of VALID_DATA events.  The argument bufferSize
 *specifies the number of bytes to be sent.  The actual number of bytes
 *transferred is returned in transferCount.  If sleep is true, then this
 *method will sleep until all bytes can be transferred.  If sleep is
 *false, then as many bytes as possible will be copied to the TX queue.
 *
 *Note that the caller should ALWAYS check the transferCount unless the
 *return value was IO_R_INVALID_ARG, indicating one or more arguments
 *were not valid.  Other possible return values are IO_R_SUCCESS if all
 *requirements were met; IO_R_IPC_FAILURE if sleep was interrupted by
 *a signal; IO_R_IO if the port was deactivated.
 */
IOReturn TIUMPSerial::
enqueueData(UInt8 *buffer, UInt32 size, UInt32 *count, bool sleep, void *refCon)
{
    DLOG("enqueueData\n");

	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;

    TIUMPPort *port = (TIUMPPort *) refCon;
	    
    if (port==NULL)
        return kIOReturnBadArgument;
    
    return port->EnqueueData(buffer, size, count, sleep);
}        

/* dequeueData will attempt to copy data from the RX queue to the specified
 *buffer.  No more than bufferSize VALID_DATA events will be transferred.
 *In other words, copying will continue until either a non-data event is
 *encountered or the transfer buffer is full.  The actual number of bytes
 *transferred is returned in transferCount.
 *
 *The sleep semantics of this method are slightly more complicated than
 *other methods in this API:  Basically, this method will continue to
 *sleep until either minCount characters have been received or a non
 *data event is next in the RX queue.  If minCount is zero, then this
 *method never sleeps and will return immediatly if the queue is empty.
 *
 *Note that the caller should ALWAYS check the transferCount unless the
 *return value was IO_R_INVALID_ARG, indicating one or more arguments
 *were not valid.  Other possible return values are IO_R_SUCCESS if all
 *requirements were met; IO_R_IPC_FAILURE if sleep was interrupted by
 *a signal; IO_R_IO if the port was deactivated.
 */
IOReturn TIUMPSerial::
dequeueData(UInt8 *buffer, UInt32 size, UInt32 *count, UInt32 min, void *refCon)
{
    DLOG("dequeueData\n");
	
	if (fTerminate || fTerminated)
		return kIOReturnNoDevice;
		
    TIUMPPort *port = (TIUMPPort *) refCon;
	    
    if (port == NULL)
        return kIOReturnBadArgument;

    return port->DequeueData(buffer, size, count, min);
}
/* _______________________End Protocol________________________  */
