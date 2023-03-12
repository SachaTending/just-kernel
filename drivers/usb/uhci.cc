#include "string.h"
#include "malloc.h"
#include "pci.h"
#include "printf.h"
#include "port_io.h"
#include "usb.h"
#include "pit.h"

typedef struct UhciTD
{
    volatile u32 link;
    volatile u32 cs;
    volatile u32 token;
    volatile u32 buffer;

    // internal fields
    u32 tdNext;
    u8 active;
    u8 pad[11];
} UhciTD;

typedef struct Link
{
    struct Link *prev;
    struct Link *next;
} Link;

typedef struct UhciQH
{
    volatile u32 head;
    volatile u32 element;

    // internal fields
    UsbTransfer *transfer;
    Link qhLink;
    u32 tdHead;
    u32 active;
    u8 pad[24];
} UhciQH;

typedef struct UhciController
{
    uint ioAddr;
    u32 *frameList;
    UhciQH *qhPool;
    UhciTD *tdPool;
    UhciQH *asyncQH;
} UhciController;

#define PCI_SERIAL_USB_UHCI             0x00
#define PCI_SERIAL_USB                  0x0c03

#define MAX_QH                          8
#define MAX_TD                          32

// TD Link Pointer
#define TD_PTR_TERMINATE                (1 << 0)
#define TD_PTR_QH                       (1 << 1)
#define TD_PTR_DEPTH                    (1 << 2)

// TD Control and Status
#define TD_CS_ACTLEN                    0x000007ff
#define TD_CS_BITSTUFF                  (1 << 17)     // Bitstuff Error
#define TD_CS_CRC_TIMEOUT               (1 << 18)     // CRC/Time Out Error
#define TD_CS_NAK                       (1 << 19)     // NAK Received
#define TD_CS_BABBLE                    (1 << 20)     // Babble Detected
#define TD_CS_DATABUFFER                (1 << 21)     // Data Buffer Error
#define TD_CS_STALLED                   (1 << 22)     // Stalled
#define TD_CS_ACTIVE                    (1 << 23)     // Active
#define TD_CS_IOC                       (1 << 24)     // Interrupt on Complete
#define TD_CS_IOS                       (1 << 25)     // Isochronous Select
#define TD_CS_LOW_SPEED                 (1 << 26)     // Low Speed Device
#define TD_CS_ERROR_MASK                (3 << 27)     // Error counter
#define TD_CS_ERROR_SHIFT               27
#define TD_CS_SPD                       (1 << 29)     // Short Packet Detect

// TD Token
#define TD_TOK_PID_MASK                 0x000000ff    // Packet Identification
#define TD_TOK_DEVADDR_MASK             0x00007f00    // Device Address
#define TD_TOK_DEVADDR_SHIFT            8
#define TD_TOK_ENDP_MASK                00x0078000    // Endpoint
#define TD_TOK_ENDP_SHIFT               15
#define TD_TOK_D                        0x00080000    // Data Toggle
#define TD_TOK_D_SHIFT                  19
#define TD_TOK_MAXLEN_MASK              0xffe00000    // Maximum Length
#define TD_TOK_MAXLEN_SHIFT             21

#define TD_PACKET_IN                    0x69
#define TD_PACKET_OUT                   0xe1
#define TD_PACKET_SETUP                 0x2d

#define REG_CMD                         0x00        // USB Command
#define REG_STS                         0x02        // USB Status
#define REG_INTR                        0x04        // USB Interrupt Enable
#define REG_FRNUM                       0x06        // Frame Number
#define REG_FRBASEADD                   0x08        // Frame List Base Address
#define REG_SOFMOD                      0x0C        // Start of Frame Modify
#define REG_PORT1                       0x10        // Port 1 Status/Control
#define REG_PORT2                       0x12        // Port 2 Status/Control
#define REG_LEGSUP                      0xc0        // Legacy Support

// ------------------------------------------------------------------------------------------------
// USB Command Register

#define CMD_RS                          (1 << 0)    // Run/Stop
#define CMD_HCRESET                     (1 << 1)    // Host Controller Reset
#define CMD_GRESET                      (1 << 2)    // Global Reset
#define CMD_EGSM                        (1 << 3)    // Enter Global Suspend Resume
#define CMD_FGR                         (1 << 4)    // Force Global Resume
#define CMD_SWDBG                       (1 << 5)    // Software Debug
#define CMD_CF                          (1 << 6)    // Configure Flag
#define CMD_MAXP                        (1 << 7)    // Max Packet (0 = 32, 1 = 64)

// ------------------------------------------------------------------------------------------------
// USB Status Register

#define STS_USBINT                      (1 << 0)    // USB Interrupt
#define STS_ERROR                       (1 << 1)    // USB Error Interrupt
#define STS_RD                          (1 << 2)    // Resume Detect
#define STS_HSE                         (1 << 3)    // Host System Error
#define STS_HCPE                        (1 << 4)    // Host Controller Process Error
#define STS_HCH                         (1 << 5)    // HC Halted

// ------------------------------------------------------------------------------------------------
// USB Interrupt Enable Register

#define INTR_TIMEOUT                    (1 << 0)    // Timeout/CRC Interrupt Enable
#define INTR_RESUME                     (1 << 1)    // Resume Interrupt Enable
#define INTR_IOC                        (1 << 2)    // Interrupt on Complete Enable
#define INTR_SP                         (1 << 3)    // Short Packet Interrupt Enable

// ------------------------------------------------------------------------------------------------
// Port Status and Control Registers

#define PORT_CONNECTION                 (1 << 0)    // Current Connect Status
#define PORT_CONNECTION_CHANGE          (1 << 1)    // Connect Status Change
#define PORT_ENABLE                     (1 << 2)    // Port Enabled
#define PORT_ENABLE_CHANGE              (1 << 3)    // Port Enable Change
#define PORT_LS                         (3 << 4)    // Line Status
#define PORT_RD                         (1 << 6)    // Resume Detect
#define PORT_LSDA                       (1 << 8)    // Low Speed Device Attached
#define PORT_RESET                      (1 << 9)    // Port Reset
#define PORT_SUSP                       (1 << 12)   // Suspend
#define PORT_RWC                        (PORT_CONNECTION_CHANGE | PORT_ENABLE_CHANGE)

static inline void IoWrite16(uint port, u16 data)
{
    __asm__ volatile("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

static inline u16 IoRead16(uint port)
{
    u16 data;
    __asm__ volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void LinkInit(Link *x)
{
    x->prev = x;
    x->next = x;
}

// ------------------------------------------------------------------------------------------------
static inline void LinkAfter(Link *a, Link *x)
{
    Link *p = a;
    Link *n = a->next;
    n->prev = x;
    x->next = n;
    x->prev = p;
    p->next = x;
}

// ------------------------------------------------------------------------------------------------
static inline void LinkBefore(Link *a, Link *x)
{
    Link *p = a->prev;
    Link *n = a;
    n->prev = x;
    x->next = n;
    x->prev = p;
    p->next = x;
}

// ------------------------------------------------------------------------------------------------
static inline void LinkRemove(Link *x)
{
    Link *p = x->prev;
    Link *n = x->next;
    n->prev = p;
    p->next = n;
    x->next = 0;
    x->prev = 0;
}

// ------------------------------------------------------------------------------------------------
static inline void LinkMoveAfter(Link *a, Link *x)
{
    Link *p = x->prev;
    Link *n = x->next;
    n->prev = p;
    p->next = n;

    p = a;
    n = a->next;
    n->prev = x;
    x->next = n;
    x->prev = p;
    p->next = x;
}

// ------------------------------------------------------------------------------------------------
static inline void LinkMoveBefore(Link *a, Link *x)
{
    Link *p = x->prev;
    Link *n = x->next;
    n->prev = p;
    p->next = n;

    p = a->prev;
    n = a;
    n->prev = x;
    x->next = n;
    x->prev = p;
    p->next = x;
}

// ------------------------------------------------------------------------------------------------
static inline bool ListIsEmpty(Link *x)
{
    return x->next == x;
}

#define LinkData(link,T,m) \
    (T *)((char *)(link) - (unsigned long)(&(((T*)0)->m)))

static UhciTD *UhciAllocTD(UhciController *hc)
{
    // TODO - better memory management
    UhciTD *end = hc->tdPool + MAX_TD;
    for (UhciTD *td = hc->tdPool; td != end; ++td)
    {
        if (!td->active)
        {
            //ConsolePrint("UhciAllocTD 0x%08x\n", td);
            td->active = 1;
            return td;
        }
    }
    return 0;
}

static UhciQH *UhciAllocQH(UhciController *hc)
{
    // TODO - better memory management
    UhciQH *end = hc->qhPool + MAX_QH;
    for (UhciQH *qh = hc->qhPool; qh != end; ++qh)
    {
        if (!qh->active)
        {
            //ConsolePrint("UhciAllocQH 0x%08x\n", qh);
            qh->active = 1;
            return qh;
        }
    }
    return 0;
}
static void UhciInsertQH(UhciController *hc, UhciQH *qh)
{
    UhciQH *list = hc->asyncQH;
    UhciQH *end = LinkData(list->qhLink.prev, UhciQH, qhLink);

    qh->head = TD_PTR_TERMINATE;
    end->head = (u32)(uintptr_t)qh | TD_PTR_QH;

    LinkBefore(&list->qhLink, &qh->qhLink);
}

static void UhciPortSet(uint port, u16 data)
{
    uint status = IoRead16(port);
    status |= data;
    status &= ~PORT_RWC;
    IoWrite16(port, status);
}

static void UhciPortClr(uint port, u16 data)
{
    uint status = IoRead16(port);
    status &= ~PORT_RWC;
    status &= ~data;
    status |= PORT_RWC & data;
    IoWrite16(port, status);
}

static uint UhciResetPort(UhciController *hc, uint port)
{
    
    uint reg = REG_PORT1 + port * 2;

    // Reset the port
    UhciPortSet(hc->ioAddr + reg, PORT_RESET);
    PitWait(50);
    UhciPortClr(hc->ioAddr + reg, PORT_RESET);

    // Wait 100ms for port to enable (TODO - what is appropriate length of time?)
    uint status = 0;
    for (uint i = 0; i < 10; ++i)
    {
        //printf("%d\r", port);
        // Delay
        PitWait(10);

        // Get current status
        status = IoRead16(hc->ioAddr + reg);

        // Check if device is attached to port
        if (~status & PORT_CONNECTION)
        {
            break;
        }

        // Acknowledge change in status
        if (status & (PORT_ENABLE_CHANGE | PORT_CONNECTION_CHANGE))
        {
            UhciPortClr(hc->ioAddr + reg, PORT_ENABLE_CHANGE | PORT_CONNECTION_CHANGE);
            continue;
        }

        // Check if device is enabled
        if (status & PORT_ENABLE)
        {
            break;
        }

        // Enable the port
        UhciPortSet(hc->ioAddr + reg, PORT_ENABLE);
        printf("UHCI: Port %d enabled.\n", i);
    }

    return status;
}

static void UhciInitTD(UhciTD *td, UhciTD *prev,
                       uint speed, uint addr, uint endp, uint toggle, uint packetType,
                       uint len, const void *data)
{
    len = (len - 1) & 0x7ff;

    if (prev)
    {
        prev->link = (u32)(uintptr_t)td | TD_PTR_DEPTH;
        prev->tdNext = (u32)(uintptr_t)td;
    }

    td->link = TD_PTR_TERMINATE;
    td->tdNext = 0;

    td->cs = (3 << TD_CS_ERROR_SHIFT) | TD_CS_ACTIVE;
    if (speed == USB_LOW_SPEED)
    {
        td->cs |= TD_CS_LOW_SPEED;
    }

    td->token =
        (len << TD_TOK_MAXLEN_SHIFT) |
        (toggle << TD_TOK_D_SHIFT) |
        (endp << TD_TOK_ENDP_SHIFT) |
        (addr << TD_TOK_DEVADDR_SHIFT) |
        packetType;

    td->buffer = (u32)(uintptr_t)data;
}

static void UhciInitQH(UhciQH *qh, UsbTransfer *t, UhciTD *td)
{
    qh->transfer = t;
    qh->tdHead = (u32)(uintptr_t)td;
    qh->element = (u32)(uintptr_t)td;
}

static void UhciRemoveQH(UhciQH *qh)
{
    UhciQH *prev = LinkData(qh->qhLink.prev, UhciQH, qhLink);

    prev->head = qh->head;
    LinkRemove(&qh->qhLink);
}

static void UhciFreeQH(UhciQH *qh)
{
    //ConsolePrint("UhciFreeQH 0x%08x\n", qh);
    qh->active = 0;
}

static void UhciFreeTD(UhciTD *td)
{
    //ConsolePrint("UhciFreeTD 0x%08x\n", td);
    td->active = 0;
}

static void UhciProcessQH(UhciController *hc, UhciQH *qh)
{
    UsbTransfer *t = qh->transfer;

    UhciTD *td = (UhciTD *)(uintptr_t)(qh->element & ~0xf);
    if (!td)
    {
        t->success = true;
        t->complete = true;
    }
    else if (~td->cs & TD_CS_ACTIVE)
    {
        if (td->cs & TD_CS_NAK)
        {
            printf("UHCI: NAK\n");
        }

        if (td->cs & TD_CS_STALLED)
        {
            printf("UHCI: TD is stalled\n");
            t->success = false;
            t->complete = true;
        }

        if (td->cs & TD_CS_DATABUFFER)
        {
            printf("UHCI: TD data buffer error\n");
        }
        if (td->cs & TD_CS_BABBLE)
        {
            printf("UHCI: TD babble error\n");
        }
        if (td->cs & TD_CS_CRC_TIMEOUT)
        {
            printf("UHCI: TD timeout error\n");
        }
        if (td->cs & TD_CS_BITSTUFF)
        {
            printf("UHCI: TD bitstuff error\n");
        }
    }

    if (t->complete)
    {
        // Clear transfer from queue
        qh->transfer = 0;

        // Update endpoint toggle state
        if (t->success && t->endp)
        {
            t->endp->toggle ^= 1;
        }

        // Remove queue from schedule
        UhciRemoveQH(qh);

        // Free transfer descriptors
        UhciTD *td = (UhciTD *)(uintptr_t)qh->tdHead;
        while (td)
        {
            UhciTD *next = (UhciTD *)(uintptr_t)td->tdNext;
            UhciFreeTD(td);
            td = next;
        }

        // Free queue head
        UhciFreeQH(qh);
    }
}

static void UhciWaitForQH(UhciController *hc, UhciQH *qh)
{
    UsbTransfer *t = qh->transfer;

    while (!t->complete)
    {
        UhciProcessQH(hc, qh);
    }
}

static void UhciDevControl(UsbDevice *dev, UsbTransfer *t)
{
    UhciController *hc = (UhciController *)dev->hc;
    UsbDevReq *req = t->req;

    // Determine transfer properties
    uint speed = dev->speed;
    uint addr = dev->addr;
    uint endp = 0;
    uint maxSize = dev->maxPacketSize;
    uint type = req->type;
    uint len = req->len;

    // Create queue of transfer descriptors
    UhciTD *td = UhciAllocTD(hc);
    if (!td)
    {
        return;
    }

    UhciTD *head = td;
    UhciTD *prev = 0;

    // Setup packet
    uint toggle = 0;
    uint packetType = TD_PACKET_SETUP;
    uint packetSize = sizeof(UsbDevReq);
    UhciInitTD(td, prev, speed, addr, endp, toggle, packetType, packetSize, req);
    prev = td;

    // Data in/out packets
    packetType = type & RT_DEV_TO_HOST ? TD_PACKET_IN : TD_PACKET_OUT;

    u8 *it = (u8 *)t->data;
    u8 *end = it + len;
    while (it < end)
    {
        td = UhciAllocTD(hc);
        if (!td)
        {
            return;
        }

        toggle ^= 1;
        packetSize = end - it;
        if (packetSize > maxSize)
        {
            packetSize = maxSize;
        }

        UhciInitTD(td, prev, speed, addr, endp, toggle, packetType, packetSize, it);

        it += packetSize;
        prev = td;
    }

    // Status packet
    td = UhciAllocTD(hc);
    if (!td)
    {
        return;
    }

    toggle = 1;
    packetType = type & RT_DEV_TO_HOST ? TD_PACKET_OUT : TD_PACKET_IN;
    UhciInitTD(td, prev, speed, addr, endp, toggle, packetType, 0, 0);

    // Initialize queue head
    UhciQH *qh = UhciAllocQH(hc);
    UhciInitQH(qh, t, head);

    // Wait until queue has been processed
    UhciInsertQH(hc, qh);
    UhciWaitForQH(hc, qh);
}

static void UhciDevIntr(UsbDevice *dev, UsbTransfer *t)
{
    UhciController *hc = (UhciController *)dev->hc;

    // Determine transfer properties
    uint speed = dev->speed;
    uint addr = dev->addr;
    uint endp = dev->endp.desc.addr & 0xf;

    // Create queue of transfer descriptors
    UhciTD *td = UhciAllocTD(hc);
    if (!td)
    {
        t->success = false;
        t->complete = true;
        return;
    }

    UhciTD *head = td;
    UhciTD *prev = 0;

    // Data in/out packets
    uint toggle = dev->endp.toggle;
    uint packetType = TD_PACKET_IN;
    uint packetSize = t->len;

    UhciInitTD(td, prev, speed, addr, endp, toggle, packetType, packetSize, t->data);

    // Initialize queue head
    UhciQH *qh = UhciAllocQH(hc);
    UhciInitQH(qh, t, head);

    // Schedule queue
    UhciInsertQH(hc, qh);
}

static void UhciProbe(UhciController *hc)
{
    printf("UCHI: Probing...\n");
    // Port setup
    uint portCount = 2;    // TODO detect number of ports
    for (uint port = 0; port < portCount; ++port)
    {
        // Reset port
        printf("UHCI: Resetting port %d...\n", port);
        uint status = UhciResetPort(hc, port);

        if (status & PORT_ENABLE)
        {
            uint speed = (status & PORT_LSDA) ? USB_LOW_SPEED : USB_FULL_SPEED;
            
            UsbDevice *dev = UsbDevCreate();
            if (dev)
            {
                dev->parent = 0;
                dev->hc = hc;
                dev->port = port;
                dev->speed = speed;
                dev->maxPacketSize = 8;

                dev->hcControl = UhciDevControl;
                dev->hcIntr = UhciDevIntr;

                if (!UsbDevInit(dev))
                {
                    // TODO - cleanup
                }
            }
        }
    }
}

void UhciInit(u32 bus, u32 dev, u32 func, PCIDevHeader *info)
{
    //printf("UHCI: Initializing UHCI on bus %d dev %d func %d...\n", bus, dev, func);
    if (info->option.classCode == PCI_SERIAL_USB &&
        info->option.progIF == PCI_SERIAL_USB_UHCI)
    {
        return;
    }

    // Base I/O Address
    PciBar bar;
    PciGetBar(&bar, bus, dev, func, 4);
    if (~bar.flags & PCI_BAR_IO)
    {
        // Only Port I/O supported
        return;
    }

    uint ioAddr = bar.u.port;
    printf("UHCI: Initializing UHCI on bus %d dev %d func %d...\n", bus, dev, func);
    // Controller initialization
    UhciController *hc = (UhciController *) malloc(sizeof(UhciController));
    hc->ioAddr = ioAddr;
    hc->frameList = (u32 *)malloc(1024 * sizeof(u32));
    hc->qhPool = (UhciQH *)malloc(sizeof(UhciQH) * MAX_QH);
    hc->tdPool = (UhciTD *)malloc(sizeof(UhciTD) * MAX_TD);

    memset(hc->qhPool, 0, sizeof(UhciQH) * MAX_QH);
    memset(hc->tdPool, 0, sizeof(UhciTD) * MAX_TD);

    // Frame list setup
    UhciQH *qh = UhciAllocQH(hc);
    qh->head = TD_PTR_TERMINATE;
    qh->element = TD_PTR_TERMINATE;
    qh->transfer = 0;
    qh->qhLink.prev = &qh->qhLink;
    qh->qhLink.next = &qh->qhLink;

    hc->asyncQH = qh;
    for (uint i = 0; i < 1024; ++i)
    {
        hc->frameList[i] = TD_PTR_QH | (u32)(uintptr_t)qh;
    }

    // Disable Legacy Support
    outportl(hc->ioAddr + REG_LEGSUP, 0x8f00);

    // Disable interrupts
    IoWrite16(hc->ioAddr + REG_INTR, 0);

    // Assign frame list
    IoWrite16(hc->ioAddr + REG_FRNUM, 0);
    outportl(hc->ioAddr + REG_FRBASEADD, (u32)(uintptr_t)hc->frameList);
    IoWrite16(hc->ioAddr + REG_SOFMOD, 0x40);

    // Clear status
    IoWrite16(hc->ioAddr + REG_STS, 0xffff);

    // Enable controller
    IoWrite16(hc->ioAddr + REG_CMD, CMD_RS);

    // Probe devices
    UhciProbe(hc);

    // Register controller
    UsbController *controller = (UsbController *)malloc(sizeof(UsbController));
    //controller->next = g_usbControllerList;
    //controller->hc = hc;
    //controller->poll = UhciControllerPoll;

    //g_usbControllerList = controller;
}