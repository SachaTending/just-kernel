#include "pci.h"
#include "malloc.h"
#include "printf.h"
#include "port_io.h"
#include "malloc.h"

#define PCI_CONFIG_PORT      0x0CF8
#define PCI_DATA_PORT        0x0CFC

#define PCI_MAX_BUSES        255
#define PCI_MAX_DEVICES      32
#define PCI_MAX_FUNCTIONS    8

#define PCI_HEADERTYPE_NORMAL        0
#define PCI_HEADERTYPE_BRIDGE        1
#define PCI_HEADERTYPE_CARDBUS       2
#define PCI_HEADERTYPE_MULTIFUNC     0x80

typedef uint32_t u32;
typedef uint8_t u8;
typedef uint16_t u16;

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

typedef struct 
{
	u32 class_code;
	char name[32];
} PCIClassName;

static PCIClassName g_PCIClassNames[] = 
{
	{ 0x00, "before PCI 2.0"},
	{ 0x01, "disk controller"},
	{ 0x02, "network interface"},
	{ 0x03, "graphics adapter"},
	{ 0x04, "multimedia controller"},
	{ 0x05, "memory controller"},
	{ 0x06, "bridge device"},
	{ 0x07, "communication controller"},
	{ 0x08, "system device"},
	{ 0x09, "input device"},
	{ 0x0a, "docking station"},
	{ 0x0b, "CPU"},
	{ 0x0c, "serial bus"},
	{ 0x0d, "wireless controller"},
	{ 0x0e, "intelligent I/O controller"},
	{ 0x0f, "satellite controller"},
	{ 0x10, "encryption controller"},
	{ 0x11, "signal processing controller"},
	{ 0xFF, "proprietary device"}
};

typedef union 
{ 
	u32 zero 		: 2;
	u32 reg_num     : 6;
	u32 func_num    : 3;
	u32 dev_num     : 5;
	u32 bus_num     : 8;
	u32 reserved    : 7;
	u32 enable_bit  : 1;
	u32 val;
} PCIConfigAddres;

typedef u16 ioport_t;

static inline void out32 (ioport_t port, u32 data)
{ asm volatile ("outl %%eax, %%dx" : : "a" (data), "d" (port)); }

static inline void in32 (ioport_t port, u32 *data)
{ asm volatile ("inl %%dx, %%eax" : "=a" (*data) : "d" (port)); }

uint32_t ReadConfig32(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset)
{
	uint64_t address;
    uint64_t lbus = (uint64_t)bus;
    uint64_t lslot = (uint64_t)slot;
    uint64_t lfunc = (uint64_t)func;
    uint32_t tmp = 0;
    address = (uint64_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outportl (0xCF8, address);
    tmp = ((inportl (0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

uint32_t PCI::pci_read_word(u32 bus, u32 dev, u32 func, u32 reg)
{
    return ReadConfig32(bus, dev, func, reg);
}

uint32_t PCI::getDeviceID(u32 bus, u32 device, u32 function)
{
        uint32_t r0 = pci_read_word(bus,device,function,2);
        return r0;
}

uint32_t PCI::getClassId(uint32_t bus, uint32_t device, uint32_t function)
{
        uint32_t r0 = pci_read_word(bus,device,function,0xA);
        return (r0 & ~0x00FF) >> 8;
}

uint32_t PCI::getSubClassId(uint32_t bus, uint32_t device, uint32_t function)
{
        uint32_t r0 = pci_read_word(bus,device,function,0xA);
        return (r0 & ~0xFF00);
}

char *getVenIDFromDB(u16 venID)
{
    switch (venID) {
        case 0x8086 :{
            return "Intel";
        };
        case 0x1234 :{
            return "Technical";
        }
        default :{
            return "Unknown";
        };
    }
}

char *GetPCIDevClassName(u32 class_code)
{
	int i;
	for (i = 0; i < sizeof(g_PCIClassNames)/sizeof(g_PCIClassNames[0]); i++)
	{
		if (g_PCIClassNames[i].class_code == class_code)
			return g_PCIClassNames[i].name;
	}
	return 0;
}

int ReadPCIDevHeader(u32 bus, u32 dev, u32 func, PCIDevHeader *p_pciDevice)
{
	int i;
	PCI pci;
	if (p_pciDevice == 0)
		return 1;
	
	for (i = 0; i < sizeof(p_pciDevice->header)/sizeof(p_pciDevice->header[0]); i++)
		p_pciDevice->header[i] = ReadConfig32(bus, dev, func, i);
		
	if (p_pciDevice->option.vendorID == 0x0000 || 
		p_pciDevice->option.vendorID == 0xffff ||
		p_pciDevice->option.deviceID == 0xffff)
		return 1;
	p_pciDevice->option.deviceID = pci.getDeviceID(bus, dev, func);
    p_pciDevice->option.classCode = pci.getClassId(bus, dev, func);
	return 0;
}

void PrintPCIDevHeader(u32 bus, u32 dev, u32 func, PCIDevHeader *p_pciDevice)
{
	char *class_name;
	char *venID_str = getVenIDFromDB(p_pciDevice->option.vendorID);
	printf("bus=0x%x dev=0x%x func=0x%x venID=0x%x(%s) devID=0x%x",
			bus, dev, func, p_pciDevice->option.vendorID, venID_str, p_pciDevice->option.deviceID);
			
	class_name = GetPCIDevClassName(p_pciDevice->option.classCode);
	if (class_name)
		printf(" class_name=%s", class_name);
		
	printf("\n");
}

void PCI::pci_proc_dump()
{


}

void PCI::pci_init()
{
    printf("PCI: Probing...\n");	
    int bus;
	int dev;
	
	for (bus = 0; bus < PCI_MAX_BUSES; bus++)
		for (dev = 0; dev < PCI_MAX_DEVICES; dev++)
		{
			u32 func = 0;
			PCIDevHeader pci_device;
			
			if (ReadPCIDevHeader(bus, dev, func, &pci_device))
				continue;
				
			PrintPCIDevHeader(bus, dev, func, &pci_device);
			
			if (pci_device.option.headerType & PCI_HEADERTYPE_MULTIFUNC)
			{
				for (func = 1; func < PCI_MAX_FUNCTIONS; func++)
				{
					if (ReadPCIDevHeader(bus, dev, func, &pci_device))
						continue;
					PrintPCIDevHeader(bus, dev, func, &pci_device);
				}
			}
		}
}