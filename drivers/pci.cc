#include "pci.h"
#include "malloc.h"
#include "printf.h"
#include "port_io.h"

pci_device **pci_devices = 0;
uint32_t devs = 0;

pci_driver **pci_drivers = 0;
uint32_t drivs = 0;

uint16_t PCI::pci_read_word(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset)
{
	uint64_t address;
    uint64_t lbus = (uint64_t)bus;
    uint64_t lslot = (uint64_t)slot;
    uint64_t lfunc = (uint64_t)func;
    uint16_t tmp = 0;
    address = (uint64_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outportl (0xCF8, address);
    tmp = (uint16_t)((inportl (0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

void PCI::pci_init()
{
	devs = drivs = 0;
	pci_devices = (pci_device **)malloc(32 * sizeof(pci_device));
	pci_drivers = (pci_driver **)malloc(32 * sizeof(pci_driver));
	PCI::pci_probe();
}

void PCI::add_pci_device(pci_device *pdev)
{
	pci_devices[devs] = pdev;
	devs ++;
	return;
}


uint16_t PCI::getVendorID(uint16_t bus, uint16_t device, uint16_t function)
{
        uint32_t r0 = PCI::pci_read_word(bus,device,function,0);
        return r0;
}

uint16_t PCI::getDeviceID(uint16_t bus, uint16_t device, uint16_t function)
{
        uint32_t r0 = PCI::pci_read_word(bus,device,function,2);
        return r0;
}

void PCI::pci_register_driver(pci_driver *driv)
{
	pci_drivers[drivs] = driv;
	drivs ++;
	return;
}

void PCI::pci_proc_dump()
{
	for(int i = 0; i < devs; i++)
	{
		pci_device *pci_dev = pci_devices[i];
		if(pci_dev->driver)
			printf("PCI: [%x:%x:%x] => %s\n", pci_dev->vendor, pci_dev->device, pci_dev->func, pci_dev->driver->name);
		else
			printf("PCI: [%x:%x:%x]\n", pci_dev->vendor, pci_dev->device, pci_dev->func);
	}
}

void PCI::pci_probe()
{
	for(uint32_t bus = 0; bus < 256; bus++)
    {
        for(uint32_t slot = 0; slot < 32; slot++)
        {
            for(uint32_t function = 0; function < 8; function++)
            {
                    uint16_t vendor = PCI::getVendorID(bus, slot, function);
                    if(vendor == 0xffff) continue;
                    uint16_t device = PCI::getDeviceID(bus, slot, function);
                    //printf("vendor: 0x%x device: 0x%x\n", vendor, device);
                    pci_device *pdev = (pci_device *)malloc(sizeof(pci_device));
                    pdev->vendor = vendor;
                    pdev->device = device;
                    pdev->func = function;
                    pdev->driver = 0;
                    PCI::add_pci_device(pdev);
            }
        }
    }
}