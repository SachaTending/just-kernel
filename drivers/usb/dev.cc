#include "malloc.h"
#include "usb.h"

UsbDevice *g_usbDeviceList;

UsbDevice *UsbDevCreate()
{
    // Initialize structure
    UsbDevice *dev = (UsbDevice *)malloc(sizeof(UsbDevice));
    if (dev)
    {
        dev->parent = 0;
        dev->next = g_usbDeviceList;
        dev->hc = 0;
        dev->drv = 0;

        dev->port = 0;
        dev->speed = 0;
        dev->addr = 0;
        dev->maxPacketSize = 0;
        dev->endp.toggle = 0;

        dev->hcControl = 0;
        dev->hcIntr = 0;
        dev->drvPoll = 0;

        g_usbDeviceList = dev;
    }

    return dev;
}