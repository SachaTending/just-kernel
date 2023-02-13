#ifndef _PACKET_H
#define _PACKET_H

#include "stdint.h"
#include "stddef.h"
#include "list.h"
#include "hash.h"
#include "spinlock.h"

struct net_info {
    uint8_t ni_hw_mac[6]; /* our HW/MAC address */
    uint32_t ni_src_ip; /* our IPv4 address */
    int ni_dhcp_state; /* where are we in the DHCP state machine */
    int ni_dhcp_tries; /* how many tries we have left to successfully do DHCP */
    int ni_arp_kick;  /* whether ARP requests should be replied */
    struct hash ni_tcp_infos; /* hashtable (key: ti_src_port) containing the
                                 current state of TCP connections
                               */
    struct hash ni_udp_sockets; /* hashtable (key: usp_src_port) containing
                                    the private state of the socket
                                */
    spinlock_t ni_tcp_infos_lock; /* lock for the table */
};

typedef struct {
    void *p_buf; // base pointer
    void *p_ptr; // Current header ptr
    uint32_t p_len; // length of the packet
    uintptr_t pkt_ip_offset;
    uintptr_t pkt_proto_offset;
    uintptr_t pkt_payload_offset;
} packet_t;

typedef struct net_device {
    struct net_info ndev_ni;

    int ndev_flags;

    int (*send_packet)(struct net_device *, packet_t *);
    int (*up)(struct net_device *);
    int (*down)(struct net_device *);

    struct list_elem elem;
} net_device_t;

#endif