#pragma once
#include "stdint.h"


// ------------------------------------------------------------------------------------------------
// USB Limits

#define USB_STRING_SIZE                 127

// ------------------------------------------------------------------------------------------------
// USB Speeds

#define USB_FULL_SPEED                  0x00
#define USB_LOW_SPEED                   0x01
#define USB_HIGH_SPEED                  0x02

typedef struct UsbIntfDesc
{
    u8 len;
    u8 type;
    u8 intfIndex;
    u8 altSetting;
    u8 endpCount;
    u8 intfClass;
    u8 intfSubClass;
    u8 intfProtocol;
    u8 intfStr;
} __attribute__((packed)) UsbIntfDesc;



typedef struct UsbDevReq
{
    u8 type;
    u8 req;
    u16 value;
    u16 index;
    u16 len;
} __attribute__((packed)) UsbDevReq;

typedef struct UsbEndpDesc
{
    u8 len;
    u8 type;
    u8 addr;
    u8 attributes;
    u16 maxPacketSize;
    u8 interval;
} __attribute__((packed)) UsbEndpDesc;

typedef struct UsbEndpoint
{
    UsbEndpDesc desc;
    uint toggle;
} UsbEndpoint;


typedef struct UsbTransfer
{
    UsbEndpoint *endp;
    UsbDevReq *req;
    void *data;
    uint len;
    bool complete;
    bool success;
} UsbTransfer;

typedef struct UsbDevice
{
    struct UsbDevice *parent;
    struct UsbDevice *next;
    void *hc;
    void *drv;

    uint port;
    uint speed;
    uint addr;
    uint maxPacketSize;

    UsbEndpoint endp;

    UsbIntfDesc intfDesc;

    void (*hcControl)(struct UsbDevice *dev, UsbTransfer *t);
    void (*hcIntr)(struct UsbDevice *dev, UsbTransfer *t);

    void (*drvPoll)(struct UsbDevice *dev);
} UsbDevice;

typedef struct UsbController
{
    struct UsbController *next;
    void *hc;

    void (*poll)(struct UsbController *controller);
} UsbController;

extern UsbController *g_usbControllerList;
extern UsbDevice *g_usbDeviceList;

#define USB_DESC_DEVICE                 0x01
#define USB_DESC_CONF                   0x02
#define USB_DESC_STRING                 0x03
#define USB_DESC_INTF                   0x04
#define USB_DESC_ENDP                   0x05

// ------------------------------------------------------------------------------------------------
// USB HID Descriptor Types

#define USB_DESC_HID                    0x21
#define USB_DESC_REPORT                 0x22
#define USB_DESC_PHYSICAL               0x23

// ------------------------------------------------------------------------------------------------
// USB HUB Descriptor Types

#define USB_DESC_HUB                    0x29

#define HUB_POWER_MASK                  0x03        // Logical Power Switching Mode
#define HUB_POWER_GLOBAL                0x00
#define HUB_POWER_INDIVIDUAL            0x01
#define HUB_COMPOUND                    0x04        // Part of a Compound Device
#define HUB_CURRENT_MASK                0x18        // Over-current Protection Mode
#define HUB_TT_TTI_MASK                 0x60        // TT Think Time
#define HUB_PORT_INDICATORS             0x80        // Port Indicators

// ------------------------------------------------------------------------------------------------
// USB Request Type

#define RT_TRANSFER_MASK                0x80
#define RT_DEV_TO_HOST                  0x80
#define RT_HOST_TO_DEV                  0x00

#define RT_TYPE_MASK                    0x60
#define RT_STANDARD                     0x00
#define RT_CLASS                        0x20
#define RT_VENDOR                       0x40

#define RT_RECIPIENT_MASK               0x1f
#define RT_DEV                          0x00
#define RT_INTF                         0x01
#define RT_ENDP                         0x02
#define RT_OTHER                        0x03

// ------------------------------------------------------------------------------------------------
// USB Device Requests

#define REQ_GET_STATUS                  0x00
#define REQ_CLEAR_FEATURE               0x01
#define REQ_SET_FEATURE                 0x03
#define REQ_SET_ADDR                    0x05
#define REQ_GET_DESC                    0x06
#define REQ_SET_DESC                    0x07
#define REQ_GET_CONF                    0x08
#define REQ_SET_CONF                    0x09
#define REQ_GET_INTF                    0x0a
#define REQ_SET_INTF                    0x0b
#define REQ_SYNC_FRAME                  0x0c

// ------------------------------------------------------------------------------------------------
// USB Hub Class Requests

#define REQ_CLEAR_TT_BUFFER             0x08
#define REQ_RESET_TT                    0x09
#define REQ_GET_TT_STATE                0x0a
#define REQ_STOP_TT                     0x0b

// ------------------------------------------------------------------------------------------------
// USB HID Interface Requests

#define REQ_GET_REPORT                  0x01
#define REQ_GET_IDLE                    0x02
#define REQ_GET_PROTOCOL                0x03
#define REQ_SET_REPORT                  0x09
#define REQ_SET_IDLE                    0x0a
#define REQ_SET_PROTOCOL                0x0b

// ------------------------------------------------------------------------------------------------
// USB Standard Feature Selectors

#define F_DEVICE_REMOTE_WAKEUP          1   // Device
#define F_ENDPOINT_HALT                 2   // Endpoint
#define F_TEST_MODE                     3   // Device

// ------------------------------------------------------------------------------------------------
// USB Hub Feature Seletcors

#define F_C_HUB_LOCAL_POWER             0   // Hub
#define F_C_HUB_OVER_CURRENT            1   // Hub
#define F_PORT_CONNECTION               0   // Port
#define F_PORT_ENABLE                   1   // Port
#define F_PORT_SUSPEND                  2   // Port
#define F_PORT_OVER_CURRENT             3   // Port
#define F_PORT_RESET                    4   // Port
#define F_PORT_POWER                    8   // Port
#define F_PORT_LOW_SPEED                9   // Port
#define F_C_PORT_CONNECTION             16  // Port
#define F_C_PORT_ENABLE                 17  // Port
#define F_C_PORT_SUSPEND                18  // Port
#define F_C_PORT_OVER_CURRENT           19  // Port
#define F_C_PORT_RESET                  20  // Port
#define F_PORT_TEST                     21  // Port
#define F_PORT_INDICATOR                22  // Port

typedef struct UsbDeviceDesc
{
    u8 len;
    u8 type;
    u16 usbVer;
    u8 devClass;
    u8 devSubClass;
    u8 devProtocol;
    u8 maxPacketSize;
    u16 vendorId;
    u16 productId;
    u16 deviceVer;
    u8 vendorStr;
    u8 productStr;
    u8 serialStr;
    u8 confCount;
} __attribute__((packed)) UsbDeviceDesc;

typedef struct UsbDriver
{
    bool (*init)(UsbDevice *dev);
} UsbDriver;

typedef struct UsbConfDesc
{
    u8 len;
    u8 type;
    u16 totalLen;
    u8 intfCount;
    u8 confValue;
    u8 confStr;
    u8 attributes;
    u8 maxPower;
} __attribute__((packed)) UsbConfDesc;

typedef struct UsbStringDesc
{
    u8 len;
    u8 type;
    u16 str[];
} __attribute__((packed)) UsbStringDesc;

UsbDevice *UsbDevCreate();
bool UsbDevInit(UsbDevice *dev);

bool UsbDevRequest(UsbDevice *dev,
    uint type, uint request,
    uint value, uint index,
    uint len, void *data);
bool UsbDevGetLangs(UsbDevice *dev, u16 *langs);