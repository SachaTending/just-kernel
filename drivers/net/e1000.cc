#include "stdint.h"
#include "e1000.h"
#include "printf2.h"

#define INTEL_VEND     0x8086  // Vendor ID for Intel 
#define E1000_DEV      0x100E  // Device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated NICs
#define E1000_I217     0x153A  // Device ID for Intel I217
#define E1000_82577LM  0x10EA  // Device ID for Intel 82577LM
 
#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_IMASK       0x00D0
#define REG_RCTRL       0x0100
#define REG_RXDESCLO    0x2800
#define REG_RXDESCHI    0x2804
#define REG_RXDESCLEN   0x2808
#define REG_RXDESCHEAD  0x2810
#define REG_RXDESCTAIL  0x2818
 
#define REG_TCTRL       0x0400
#define REG_TXDESCLO    0x3800
#define REG_TXDESCHI    0x3804
#define REG_TXDESCLEN   0x3808
#define REG_TXDESCHEAD  0x3810
#define REG_TXDESCTAIL  0x3818
 
#define REG_RDTR         0x2820 // RX Delay Timer Register
#define REG_RXDCTL       0x3828 // RX Descriptor Control
#define REG_RADV         0x282C // RX Int. Absolute Delay Timer
#define REG_RSRPD        0x2C00 // RX Small Packet Detect Interrupt
 
 
 
#define REG_TIPG         0x0410      // Transmit Inter Packet Gap
#define ECTRL_SLU        0x40        //set link up
 
 
#define RCTL_EN                         (1 << 1)    // Receiver Enable
#define RCTL_SBP                        (1 << 2)    // Store Bad Packets
#define RCTL_UPE                        (1 << 3)    // Unicast Promiscuous Enabled
#define RCTL_MPE                        (1 << 4)    // Multicast Promiscuous Enabled
#define RCTL_LPE                        (1 << 5)    // Long Packet Reception Enable
#define RCTL_LBM_NONE                   (0 << 6)    // No Loopback
#define RCTL_LBM_PHY                    (3 << 6)    // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF                 (0 << 8)    // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER              (1 << 8)    // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH               (2 << 8)    // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36                      (0 << 12)   // Multicast Offset - bits 47:36
#define RCTL_MO_35                      (1 << 12)   // Multicast Offset - bits 46:35
#define RCTL_MO_34                      (2 << 12)   // Multicast Offset - bits 45:34
#define RCTL_MO_32                      (3 << 12)   // Multicast Offset - bits 43:32
#define RCTL_BAM                        (1 << 15)   // Broadcast Accept Mode
#define RCTL_VFE                        (1 << 18)   // VLAN Filter Enable
#define RCTL_CFIEN                      (1 << 19)   // Canonical Form Indicator Enable
#define RCTL_CFI                        (1 << 20)   // Canonical Form Indicator Bit Value
#define RCTL_DPF                        (1 << 22)   // Discard Pause Frames
#define RCTL_PMCF                       (1 << 23)   // Pass MAC Control Frames
#define RCTL_SECRC                      (1 << 26)   // Strip Ethernet CRC
 
// Buffer Sizes
#define RCTL_BSIZE_256                  (3 << 16)
#define RCTL_BSIZE_512                  (2 << 16)
#define RCTL_BSIZE_1024                 (1 << 16)
#define RCTL_BSIZE_2048                 (0 << 16)
#define RCTL_BSIZE_4096                 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192                 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384                ((1 << 16) | (1 << 25))
 
 
// Transmit Command
 
#define CMD_EOP                         (1 << 0)    // End of Packet
#define CMD_IFCS                        (1 << 1)    // Insert FCS
#define CMD_IC                          (1 << 2)    // Insert Checksum
#define CMD_RS                          (1 << 3)    // Report Status
#define CMD_RPS                         (1 << 4)    // Report Packet Sent
#define CMD_VLE                         (1 << 6)    // VLAN Packet Enable
#define CMD_IDE                         (1 << 7)    // Interrupt Delay Enable
 
 
// TCTL Register
 
#define TCTL_EN                         (1 << 1)    // Transmit Enable
#define TCTL_PSP                        (1 << 3)    // Pad Short Packets
#define TCTL_CT_SHIFT                   4           // Collision Threshold
#define TCTL_COLD_SHIFT                 12          // Collision Distance
#define TCTL_SWXOFF                     (1 << 22)   // Software XOFF Transmission
#define TCTL_RTLC                       (1 << 24)   // Re-transmit on Late Collision
 
#define TSTA_DD                         (1 << 0)    // Descriptor Done
#define TSTA_EC                         (1 << 1)    // Excess Collisions
#define TSTA_LC                         (1 << 2)    // Late Collision
#define LSTA_TU                         (1 << 3)    // Transmit Underrun

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

uint8_t e1000_test_packet[] = 
{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* eth dest (broadcast) */
    0x52, 0x54, 0x00, 0x12, 0x34, 0x56, /* eth source */
    0x08, 0x06, /* eth type */
    0x00, 0x01, /* ARP htype */
    0x08, 0x00, /* ARP ptype */
    0x06, /* ARP hlen */
    0x04, /* ARP plen */
    0x00, 0x01, /* ARP opcode: ARP_REQUEST */
    0x52, 0x54, 0x00, 0x12, 0x34, 0x56, /* ARP hsrc */
    169, 254, 13, 37, /* ARP psrc */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ARP hdst */
    192, 168, 0, 137, /* ARP pdst */
};

void E1000::write_cmd(struct e1000_device *edev, uint16_t addr, uint32_t val)
{
    if (edev->bar_type == 0) {
        *(uint32_t *)(edev->mbase + addr) = val;
    } else {
        printf("e1000: io ports are not supported\n");
        /*Ports::outportl(io_base, p_address);
        Ports::outportl(io_base + 4, p_value);*/
    }
}

uint32_t E1000::read_cmd(struct e1000_device *edev, uint16_t addr)
{
    if (edev->bar_type == 0) {
        return *(uint32_t *)(edev->mbase + addr);
    } else {
        /*Ports::outportl(io_base, p_address);
        return Ports::inportl(io_base + 4);*/
        printf("e1000: io ports are not supported\n");
        return 0;
    }
}

uint32_t E1000::read_eeprom(struct e1000_device *edev, uint8_t addr)
{
    uint16_t data = 0;
    uint32_t tmp = 0;
    E1000::write_cmd(edev, REG_EEPROM, (1) | ((uint32_t)(addr) << 8));

    while(!((tmp = E1000::read_cmd(edev, REG_EEPROM)) & (1 << 4)));

    data = (uint16_t)((tmp >> 16) & 0xFFFF);
	return data;
}


int E1000::read_mac(struct e1000_device *edev, uint8_t *mac)
{
    uint32_t temp;

    temp = E1000::read_eeprom(edev, 0);
    mac[0] = temp &  0xff;
    mac[1] = temp >> 8;
    temp = E1000::read_eeprom(edev, 1);
    mac[2] = temp &  0xff;
    mac[3] = temp >> 8;
    temp = E1000::read_eeprom(edev, 2);
    mac[4] = temp &  0xff;
    mac[5] = temp >> 8;
    
    return 0;
}
