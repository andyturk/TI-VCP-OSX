/*
 *UMPSerialDriver.h - This file contains the class definition for the
 *		     TIUMPSerial driver
 *		     
 *
 */

#ifndef TIUMPSerial_h
#define TIUMPSerial_h

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>

#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOConditionLock.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/platform/AppleMacIODevice.h>

#include <IOKit/serial/IOSerialDriverSync.h>

#include <IOKit/usb/IOUSBDevice.h>

#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOFilterInterruptEventSource.h>

#include <IOKit/IOBufferMemoryDescriptor.h>

#include "TIUMPPrimitives.h"

class TIUMPDevice;

#define USE_WORK_LOOPS	1
#define USE_FILTER_EVENT_SOURCES	0


#define SPECIAL_SHIFT		(5)
#define SPECIAL_MASK		((1<<SPECIAL_SHIFT) - 1)
#define STATE_ALL			(PD_RS232_S_MASK | PD_S_MASK )
#define FLOW_RX_AUTO   	 	(PD_RS232_A_RFR | PD_RS232_A_DTR \
                                | PD_RS232_A_RXO )
#define FLOW_TX_AUTO    	(PD_RS232_A_CTS | PD_RS232_A_DSR \
                                | PD_RS232_A_TXO | PD_RS232_A_DCD )
#define CAN_BE_AUTO			(FLOW_RX_AUTO | FLOW_TX_AUTO )
#define CAN_NOTIFY			(PD_RS232_N_MASK )
#define EXTERNAL_MASK   	(PD_S_MASK | (PD_RS232_S_MASK & ~PD_RS232_S_LOOP) )
#define INTERNAL_DELAY  	(PD_RS232_S_LOOP )
#define DEFAULT_AUTO		(PD_RS232_A_DTR | PD_RS232_A_RFR \
                                | PD_RS232_A_CTS | PD_RS232_A_DSR )
#define DEFAULT_NOTIFY		(0x00 )
#define DEFAULT_STATE		(PD_S_TX_ENABLE | PD_S_RX_ENABLE \
                                | PD_RS232_A_TXO | PD_RS232_A_RXO )
#define IDLE_XO	   		0
#define NEEDS_XOFF 		1
#define SENT_XOFF 		-1
#define NEEDS_XON  		2
#define SENT_XON  		-2

#define	CONTINUE_SEND	1
#define	PAUSE_SEND		2

//typedef void				*CalloutEntry;
//typedef UInt32 long		CalloutTime;
typedef UInt32			CalloutTime;


#define calloutEntryAllocate 		thread_call_allocate
#define calloutEntryRemove		thread_call_cancel
#define calloutEntryFree		thread_call_free
#define calloutEntryDispatch		thread_call_enter
#define calloutEntryDispatchDelayed 	thread_call_enter_delayed
//#define calloutDeadlineFromInterval 	deadline_from_interval *** threads

#define    kX1UserClockMode		0x00	// for MIDI devices
#define    kX16UserClockMode	0x0b
#define    kX32UserClockMode	0x19
#define    kX64UserClockMode	0x32

#include <IOKit/ppc/IODBDMA.h>

/*
 *It is possible to improve the total behavior of the driver changing the
 *dimension of the MAX_BLOCK_SIZE, that can not be however larger than
 *PAGESIZE
 */
#define MAX_BLOCK_SIZE		PAGE_SIZE

class TIUMPSerial : public IOSerialDriverSync
{
    OSDeclareDefaultStructors(TIUMPSerial)

public:

	// member variables
    IOWorkLoop*			myWorkLoop;		// holds the workloop for this driver
	IOTimerEventSource*	myTimer;		// holds the timer we create
	UInt32				counter;		// counter incremented each time the timeout handler is called

    IOUSBDevice                 *fProvider;
    TIUMPDevice                 *mUMPDevice;
    bool                        fTerminate;
    UInt8						fPowerState;    // off,on ordinal for power management
	bool						fTerminated;
  
    /* IOSerialDriverSync Implementation */
    virtual IOReturn acquirePort(bool sleep, void *refCon);
    virtual IOReturn releasePort(void *refCon);
    virtual IOReturn setState(UInt32 state, UInt32 mask, void *refCon);
    UInt32 getState(void *refCon);
    virtual IOReturn watchState(UInt32 *state, UInt32 mask, void *refCon);
    UInt32 nextEvent(void *refCon);
    virtual IOReturn executeEvent(UInt32 event, UInt32 data, void *refCon);
    virtual IOReturn requestEvent(UInt32 event, UInt32 *data, void *refCon);
    virtual IOReturn enqueueEvent(UInt32 event, UInt32 data,
                                  bool sleep, void *refCon);
    virtual IOReturn dequeueEvent(UInt32 *event, UInt32 *data,
                                  bool sleep, void *refCon);
    virtual IOReturn enqueueData(UInt8 *buffer, UInt32 size, UInt32 *count,
                                 bool sleep, void *refCon);
    virtual IOReturn dequeueData(UInt8 *buffer, UInt32 size, UInt32 *count,
                                 UInt32 min, void *refCon);
    /* End IOSerialDriverSync Implementation */

    /* IOKit methods */
    virtual bool start(IOService *provider);
	virtual bool initForPM(IOService * provider);
	virtual unsigned long initialPowerStateForDomainState ( IOPMPowerFlags flags);
	virtual IOReturn setPowerState(unsigned long powerStateOrdinal, IOService * whatDevice);
    virtual void stop(IOService *provider);
    virtual IOService* probe(IOService *provider, SInt32 *pScore);
    virtual IOReturn message( UInt32 type, IOService *provider,  void *argument);
	virtual bool willTerminate(IOService * provider, IOOptionBits options);
	virtual bool didTerminate( IOService * provider, IOOptionBits options, bool * defer );

    
    void DoTerminate(void);
};

#endif
