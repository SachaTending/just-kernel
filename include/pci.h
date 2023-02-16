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