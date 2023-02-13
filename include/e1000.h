#ifndef _E1000_H
#define _E1000_H

#include "stdint.h"
#include "pci.h"
#include "packet.h"
#include "types.h"

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

struct e1000_rx_desc {
        volatile uint64_t addr;
        volatile uint16_t length;
        volatile uint16_t checksum;
        volatile uint8_t status;
        volatile uint8_t errors;
        volatile uint16_t special;
} __packed;
 
struct e1000_tx_desc {
        volatile uint64_t addr;
        volatile uint16_t length;
        volatile uint8_t cso;
        volatile uint8_t cmd;
        volatile uint8_t status;
        volatile uint8_t css;
        volatile uint16_t special;
} __packed;

struct e1000_device {
    int bar_type;
    uint32_t mbase;

    struct e1000_rx_desc *rx_descs[E1000_NUM_RX_DESC];
    struct e1000_tx_desc *tx_descs[E1000_NUM_TX_DESC];
    uint16_t rx_cur;
    uint16_t tx_cur;

    uint8_t mac[6];

    struct net_device ndev;

    pci_device *pdev;
};

class E1000 {
    public:
        void write_cmd(struct e1000_device *edev, uint16_t addr, uint32_t val);
        uint32_t read_cmd(struct e1000_device *edev, uint16_t addr);

        uint32_t read_eeprom(struct e1000_device *edev, uint8_t addr);
        int read_mac(struct e1000_device *edev, uint8_t *mac);
};

#endif