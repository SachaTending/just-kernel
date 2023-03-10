#ifndef _PCI_H
#define _PCI_H

#include "stdint.h"

struct __pci_driver;

typedef struct {
	uint32_t vendor;
	uint32_t device;
	uint32_t func;
	struct __pci_driver *driver;
} pci_device;

typedef struct {
	uint32_t vendor;
	uint32_t device;
	uint32_t func;
} pci_device_id;

typedef struct __pci_driver {
	pci_device_id *table;
	char *name;
	uint8_t (*init_one)(pci_device *);
	uint8_t (*init_driver)(void);
	uint8_t (*exit_driver)(void);
} pci_driver;

typedef union 
{
	struct
	{
		u16 vendorID;
		u16 deviceID;
		u16 commandReg;
		u16 statusReg;
		u8 revisionID;
		u8 progIF;
		u8 subClassCode;
		u8 classCode;
		u8 cachelineSize;
		u8 latency;
		u8 headerType;
		u8 BIST;
	} __attribute__((packed)) option;
	u32 header[4];
} __attribute__((packed)) PCIDevHeader;

#define PCI_CONFIG_PORT      0x0CF8
#define PCI_DATA_PORT        0x0CFC

#define PCI_MAX_BUSES        255
#define PCI_MAX_DEVICES      32
#define PCI_MAX_FUNCTIONS    8

#define PCI_HEADERTYPE_NORMAL        0
#define PCI_HEADERTYPE_BRIDGE        1
#define PCI_HEADERTYPE_CARDBUS       2
#define PCI_HEADERTYPE_MULTIFUNC     0x80

#define PCI_MAKE_ID(bus, dev, func)     ((bus) << 16) | ((dev) << 11) | ((func) << 8)

// I/O Ports
#define PCI_CONFIG_ADDR                 0xcf8
#define PCI_CONFIG_DATA                 0xcfC

// Header Type
#define PCI_TYPE_MULTIFUNC              0x80
#define PCI_TYPE_GENERIC                0x00
#define PCI_TYPE_PCI_BRIDGE             0x01
#define PCI_TYPE_CARDBUS_BRIDGE         0x02

// PCI Configuration Registers
#define PCI_CONFIG_VENDOR_ID            0x00
#define PCI_CONFIG_DEVICE_ID            0x02
#define PCI_CONFIG_COMMAND              0x04
#define PCI_CONFIG_STATUS               0x06
#define PCI_CONFIG_REVISION_ID          0x08
#define PCI_CONFIG_PROG_INTF            0x09
#define PCI_CONFIG_SUBCLASS             0x0a
#define PCI_CONFIG_CLASS_CODE           0x0b
#define PCI_CONFIG_CACHELINE_SIZE       0x0c
#define PCI_CONFIG_LATENCY              0x0d
#define PCI_CONFIG_HEADER_TYPE          0x0e
#define PCI_CONFIG_BIST                 0x0f

// Type 0x00 (Generic) Configuration Registers
#define PCI_CONFIG_BAR0                 0x10
#define PCI_CONFIG_BAR1                 0x14
#define PCI_CONFIG_BAR2                 0x18
#define PCI_CONFIG_BAR3                 0x1c
#define PCI_CONFIG_BAR4                 0x20
#define PCI_CONFIG_BAR5                 0x24
#define PCI_CONFIG_CARDBUS_CIS          0x28
#define PCI_CONFIG_SUBSYSTEM_VENDOR_ID  0x2c
#define PCI_CONFIG_SUBSYSTEM_DEVICE_ID  0x2e
#define PCI_CONFIG_EXPANSION_ROM        0x30
#define PCI_CONFIG_CAPABILITIES         0x34
#define PCI_CONFIG_INTERRUPT_LINE       0x3c
#define PCI_CONFIG_INTERRUPT_PIN        0x3d
#define PCI_CONFIG_MIN_GRANT            0x3e
#define PCI_CONFIG_MAX_LATENCY          0x3f

#define PCI_BAR_IO                      0x01
#define PCI_BAR_LOWMEM                  0x02
#define PCI_BAR_64                      0x04
#define PCI_BAR_PREFETCH                0x08

typedef struct PciBar
{
    union
    {
        void *address;
        u16 port;
    } u;
    u64 size;
    uint flags;
} PciBar;

void PciGetBar(PciBar *bar, u32 bus, u32 dev, u32 func, uint index);

class PCI {
    public:
        void pci_init();
        void pci_probe();
        void add_pci_device(pci_device *pdev);

        uint32_t pci_read_word(uint32_t bus, uint32_t dev, uint32_t func, uint32_t reg);

        uint16_t getVendorID(uint16_t bus, uint16_t device, uint16_t function);
        uint32_t getDeviceID(uint32_t bus, uint32_t device, uint32_t function);
		uint32_t getClassId(uint32_t bus, uint32_t device, uint32_t function);
		uint32_t getSubClassId(uint32_t bus, uint32_t device, uint32_t function);

        void pci_register_driver(pci_driver *driv);

        void pci_proc_dump();
};

#endif