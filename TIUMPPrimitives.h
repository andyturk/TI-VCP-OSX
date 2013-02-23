#ifndef __TIUMPPrimitives_h__
#define __TIUMPPrimitives_h__

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
#include <IOKit/usb/IOUSBInterface.h>
#include <IOKit/usb/IOUSBPipe.h>

#include <IOKit/IORegistryEntry.h>

#ifndef macintosh
    #define macintosh
#endif

#include "FWInclude/DevFWApi.h"

#include "UMPSerialDriver.h"


//#define SHOW_DEBUG_STRINGS	
#ifdef  SHOW_DEBUG_STRINGS
#define DLOG(fmt, args...)      {IORecursiveLockLock(gDebugLock);IOLog(fmt, ## args);IORecursiveLockUnlock(gDebugLock);}
#else
#define DLOG(fmt, args...)	
#endif

class TIUMPPort;
class TIUMPSerial;

class TIUMPDevice 
{
protected:
    
    typedef enum {
        kComDirectionIn,
        kComDirectionOut
    } CommDirT;
    
public:
    typedef enum {
        kChip3410,
        kChip5052
    } ChipIDT;
    
        
    TIUMPDevice(TIUMPSerial* inService, IOUSBDevice *inUSBDevice, ChipIDT inChipID);
    
    ~TIUMPDevice();
    
	//low level USB Firmware call
    int CallFW(CommDirT inDirection, TIUMPPort *inPort, int inSelector, int inValue, void *ioBuffer, UInt32 &ioBufSize);
    
	//Upload firmware
    int UploadFirmware(void);
    
	//create suffix to make sure the device name is unique
	bool createSuffix(int i, unsigned char *sufKey);

	//enumerate ports on device and create TIUMPPort for each of one
    int EnumeratePorts(void);
    
	//returns if FW is loaded in device
    bool IsFWLoaded(void);
    
	//returns device's FW version struct
    int GetFWVersion(ptDevFwVersion outVersion);
    
	//ask device to open port
    int OpenPort(TIUMPPort *inPort);
    
	//ask device to close port
    int ClosePort(TIUMPPort *inPort);
    
	//ask device to start port
    int StartPort(TIUMPPort *inPort);
    
	//ask device to stop port
    int StopPort(TIUMPPort *inPort);
    
	//returns device's port config
    int GetPortConfig(TIUMPPort *inPort, ptUmpUartConfig outConfig);
    
	//set device's port config
    int SetPortConfig(TIUMPPort *inPort, ptUmpUartConfig inConfig);
    
	//returns actual device status
    int GetPortStatus(TIUMPPort *inPort, ptUmpPortStatus outStatus);
    
	//returns number of ports on device
    UInt32 GetPortsNum(void) {return mPortNum;}
    
	// returns ith port on device
    TIUMPPort* GetIthPort(UInt32 i) {
        if (mPortList && i<mPortNum)
            return mPortList[i];
        else
            return NULL;
    }
    
	//call back when reading interrupt pipe finishes
    static void interruptReadComplete( void *obj, void *param, IOReturn rc, UInt32 remaining );
    
	//start polling of interrupt pipe
    int StartIntPolling();
	
	//stop polling of interrupt pipe
    int StopIntPolling();
    
	//convert baudrate num to internal UART code
    WORD EncodeBaudRate(UInt32 baudRate);
	
	//stop all ports and terminate device
	void Terminate();
	
	//set terminate flag - if set, there is no device connected
	void SetTerminate(bool inTerminate) {mTerminate=inTerminate;}
	
	//make sure the structure is always bigendian aware
	void FixConfigEndianess(ptUmpUartConfig outConfig);
	
	//make sure the structure is always bigendian aware
	void FixFWVersionEndianess(ptDevFwVersion ioFWVer);
	
public:
    IOUSBDevice                 *mUSBDevice;
    ChipIDT                     mChipID;
    TIUMPSerial                 *mService;
    TIUMPPort*                  *mPortList;
    UInt32                      mPortNum;
    UInt32                      mListAllocSize;
    IOUSBPipe                   *mIntPipe;
    IOBufferMemoryDescriptor    *mIntPipeMDP;
    UInt8                       *mIntPipeBuffer;
    IOBufferMemoryDescriptor    *mControlPipeOutMDP;
    UInt8                       *mControlPipeOutBuffer;
    IOBufferMemoryDescriptor    *mControlPipeInMDP;
    UInt8                       *mControlPipeInBuffer;
    IOUSBCompletion             mIntCompletionInfo;
    bool                        mIntPipeStarted;
	bool						mTerminate;
};

class TIUMPPort
{
public:
    typedef struct
    {
        UInt8   *Start;
        UInt8   *End;
        UInt8   *NextChar;
        UInt8   *LastChar;
        size_t  Size;
        size_t  InQueue;
    } CirQueue;
    
    typedef enum
    {
        queueNoError = 0,
        queueFull,
        queueEmpty,
        queueMaxStatus
    } QueueStatus;  
    
    typedef struct BufferMarks
    {
        UInt32   BufferSize;
        UInt32   HighWater;
        UInt32   LowWater;
        bool     OverRun;
    } BufferMarks;  
	
	typedef struct OSConfig
	{
		UInt32 BaudRate;
		UInt32 CharBits;
		UInt32 StopBits;
		UInt32 Parity;
		UInt8  XONChar;
		UInt8  XOFFChar;
		UInt32 FlowControl;
		bool OverrunError;
		bool ParityError;
		bool FramingError;
		bool RXFull;
		bool TXEmpty;
	} OSConfig;
	
	typedef enum
	{
		kMskBaudrate	= 1 << 0,
		kMskCharBits	= 1 << 1,
		kMskStopBits	= 1 << 2,
		kMskParity		= 1 << 3,
		kMskXONChar		= 1 << 4,
		kMskXOFFChar	= 1 << 5,
		kMskFlowControl = 1 << 6,
		kMskState		= 1 << 7
	} OSConfigMask;
    
public:  

    TIUMPPort(TIUMPDevice *inUMPDevice, IOUSBPipe *inPipeIn, IOUSBPipe *inPipeOut, int inPortID);
    
	~TIUMPPort();
    
	//return UMP port ID
    int GetPortID() {return mPortID;}
    
	//Reload state of flow controls
    IOReturn ReloadState();
    
	//acquire port by system = open port
    IOReturn Acquire();
    
	//release port by system = close port
    IOReturn Release();
    
	//return if is acquired
    bool IsAcquired();
    
	//prepare buffers, etc.
    IOReturn AllocateResources( void );
    
	//free buffers, etc.
    IOReturn FreeResources(void);
    
	//start USB pipes, start read and interrupt pipe polling
    IOReturn StartPipes( void );
    
	//stop USB pipes, abort all transfers
    void StopPipes();
    
	//call back when USB read completes
    static void DataReadComplete( void *obj, void *param, IOReturn rc, UInt32 remaining );
    
	//call back when USB write completes
    static void DataWriteComplete( void *obj, void *param, IOReturn rc, UInt32 remaining );
    
	//add byte to ring buffer
    TIUMPPort::QueueStatus AddBytetoQueue( TIUMPPort::CirQueue *Queue, char Value );
    
	//get byte from ring buffer
    TIUMPPort::QueueStatus GetBytefromQueue( TIUMPPort::CirQueue *Queue, UInt8 *Value );
    
	//init ring buffer
    TIUMPPort::QueueStatus InitQueue( TIUMPPort::CirQueue *Queue, UInt8 *Buffer, size_t Size );
    
	//close ring buffer
    TIUMPPort::QueueStatus CloseQueue( TIUMPPort::CirQueue *Queue );
    
	//add buffer to ring buffer
    size_t AddtoQueue( TIUMPPort::CirQueue *Queue, UInt8 *Buffer, size_t Size );
    
	//remove data from ring buffer
    size_t RemovefromQueue( TIUMPPort::CirQueue *Queue, UInt8 *Buffer, size_t MaxSize );
    
	//return free space in ring buffer
    size_t FreeSpaceinQueue( TIUMPPort::CirQueue *Queue );
    
	//return used space in ring buffer
    size_t UsedSpaceinQueue( TIUMPPort::CirQueue *Queue );
    
	//return total size of ring buffer
    size_t GetQueueSize( TIUMPPort::CirQueue *Queue );
    
	//check consistency and state of ring buffer
    void CheckQueues();
    
	//copy bytes read from USB to rounf buffer
    void Add_RXBytes( UInt8 *Buffer, size_t Size );
    
	//get port state
    UInt32 ReadPortState();
    
	//set new port state
    void ChangeState( UInt32 state, UInt32 mask );  
    
	//wait expected state - this func blocks
    IOReturn WatchState(UInt32 *state, UInt32 mask);
   
	//set general default values
    void SetDefaults(void);
    
	//add data for sending to USB
    IOReturn EnqueueData(UInt8 *buffer, UInt32 size, UInt32 *count, bool sleep);
    
	//get data read from USB
    IOReturn DequeueData(UInt8 *buffer, UInt32 size, UInt32 *count, UInt32 min);
    
	//free ring buffer
    void FreeRingBuffer(CirQueue *Queue);
    
	//create ring buffer
    bool AllocateRingBuffer(CirQueue *Queue, size_t BufferSize);
    
	//set actual HW modem status/error
	void SetHWStatus(tUmpPortStatus hwState);

	//get actualHW modem status/error
	void GetHWStatus(tUmpPortStatus &hwState);

	//update status based on HW modem status
	void UpdateStatus(void);
	
	//reread status of device UART
    IOReturn ReloadStatus(void);

	//reread device UART
    IOReturn ReloadConfig(void);
    
	//start transmitting data to USB
    IOReturn SetUpTransmit( void );
	
	//set params of system config struct
	void SetOSConfig(TIUMPPort::OSConfig &cfg, UInt32 mask);
	
	//set defaults of system config struct
	void SetOSConfigDefaults(void);

	//get params of system config struct
	void GetOSConfig(TIUMPPort::OSConfig &cfg);

	//fill data asked by system
	IOReturn RequestEvent(UInt32 event, UInt32 *data);
	
	//get data set by system
	IOReturn ExecuteEvent(UInt32 event, UInt32 data);
	
	//Start port = make active, allow writting and reading data
	IOReturn Start(void);
	
	//Stop port = make inactive
	IOReturn Stop(void);
	
	//store system RS232 stream
	void SetStream(IORS232SerialStreamSync *inStream) {mRS232Stream=inStream;}
	
	//get stored system RS232 stream
	IORS232SerialStreamSync *GetStream(void) {return mRS232Stream;}
	
	//register serial Stream to posix

public:
	IORS232SerialStreamSync		*mRS232Stream;
    IOUSBPipe                   *mPipeIn;
    IOBufferMemoryDescriptor    *mPipeInMDP;
    UInt8                       *mPipeInBuffer;
    IOUSBCompletion             mReadCompletionInfo;
    bool                        mReadActive;
    CirQueue                    mRX;
    IOUSBPipe                   *mPipeOut;
    IOBufferMemoryDescriptor    *mPipeOutMDP;
    UInt8                       *mPipeOutBuffer;
    IOUSBCompletion             mWriteCompletionInfo;
    bool                        mWriteActive;
    CirQueue                    mTX;
    int                         mPortID;
    TIUMPDevice                 *mUMPDevice;
    bool                        mAcquired;
    IORecursiveLock				*mRequestLock;
	IOLock						*mWatchLock;
    UInt32                      mOSPortState;
    BufferMarks                 mRXStats;
    BufferMarks                 mTXStats;
    UInt32                      mOSFlowControl;
    UInt32                      mOSFlowControlState;
    UInt32                      mWatchStateMask;
    tUmpPortStatus              mHWPortStatus;
    tUmpUartConfig              mHWPortConfig;
    bool                        mAreTransmitting;
	TIUMPPort::OSConfig			mOSPortConfig;
};

#endif //__TIUMPPrimitives_h__