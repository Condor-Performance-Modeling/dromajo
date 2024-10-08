// -------------------------------------------------------------------------
// Copyright (c) 2016-2017 Fabrice Bellard
// Copyright (C) 2017,2018,2019, Esperanto Technologies Inc.
// Copyright (C) 2023-2024, Condor Computing Corporation
//
// See dromajo_main.cpp for license notice.
// -------------------------------------------------------------------------
// Split from the original dromajo_main
// -------------------------------------------------------------------------
#include "network.h"
#include "riscv_machine.h"

#include <net/if.h>
#include <sys/ioctl.h>

#ifndef __APPLE__
#include <linux/if_tun.h>
#endif

#ifdef CONFIG_SLIRP
#include "slirp/libslirp.h"
#endif

#ifndef __APPLE__

void tun_write_packet(EthernetDevice *net, const uint8_t *buf, int len) {
    TunState *s   = (TunState *)net->opaque;
    ssize_t   got = write(s->fd, buf, len);
    (void) got; // Make GCC happy
    assert(got == len);
}

void tun_select_fill(EthernetDevice *net, int *pfd_max, fd_set *rfds, fd_set *wfds, fd_set *efds, int *pdelay) {
    TunState *s      = (TunState *)net->opaque;
    int       net_fd = s->fd;

    s->select_filled = net->device_can_write_packet(net);
    if (s->select_filled) {
        FD_SET(net_fd, rfds);
        *pfd_max = max_int(*pfd_max, net_fd);
    }
}

void tun_select_poll(EthernetDevice *net, fd_set *rfds, fd_set *wfds, fd_set *efds, int select_ret) {
    TunState *s      = (TunState *)net->opaque;
    int       net_fd = s->fd;
    uint8_t   buf[2048];
    int       ret;

    if (select_ret <= 0)
        return;
    if (s->select_filled && FD_ISSET(net_fd, rfds)) {
        ret = read(net_fd, buf, sizeof(buf));
        if (ret > 0)
            net->device_write_packet(net, buf, ret);
    }
}

/* configure with:
# bridge configuration (connect tap0 to bridge interface br0)
   ip link add br0 type bridge
   ip tuntap add dev tap0 mode tap [user x] [group x]
   ip link set tap0 master br0
   ip link set dev br0 up
   ip link set dev tap0 up

# NAT configuration (eth1 is the interface connected to internet)
   ifconfig br0 192.168.3.1
   echo 1 > /proc/sys/net/ipv4/ip_forward
   iptables -D FORWARD 1
   iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE

   In the VM:
   ifconfig eth0 192.168.3.2
   route add -net 0.0.0.0 netmask 0.0.0.0 gw 192.168.3.1
*/

EthernetDevice *tun_open(const char *ifname) {
    struct ifreq ifr;
    int          fd, ret;

    fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        fprintf(dromajo_stderr, "Error: could not open /dev/net/tun\n");
        return NULL;
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    pstrcpy(ifr.ifr_name, sizeof(ifr.ifr_name), ifname);
    ret = ioctl(fd, TUNSETIFF, (void *)&ifr);
    if (ret != 0) {
        fprintf(dromajo_stderr, "Error: could not configure /dev/net/tun\n");
        close(fd);
        return NULL;
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);

    EthernetDevice *net = (EthernetDevice *)mallocz(sizeof *net);
    net->mac_addr[0]    = 0x02;
    net->mac_addr[1]    = 0x00;
    net->mac_addr[2]    = 0x00;
    net->mac_addr[3]    = 0x00;
    net->mac_addr[4]    = 0x00;
    net->mac_addr[5]    = 0x01;

    TunState *s       = (TunState *)mallocz(sizeof *s);
    s->fd             = fd;
    net->opaque       = s;
    net->write_packet = tun_write_packet;
    net->select_fill  = tun_select_fill;
    net->select_poll  = tun_select_poll;
    return net;
}

#endif /* !__APPLE__*/

#ifdef CONFIG_SLIRP

/*******************************************************/
/* slirp */

Slirp *slirp_state;

void slirp_write_packet(EthernetDevice *net, const uint8_t *buf, int len) {
    Slirp *slirp_state = net->opaque;
    slirp_input(slirp_state, buf, len);
}

int slirp_can_output(void *opaque) {
    EthernetDevice *net = opaque;
    return net->device_can_write_packet(net);
}

void slirp_output(void *opaque, const uint8_t *pkt, int pkt_len) {
    EthernetDevice *net = opaque;
    return net->device_write_packet(net, pkt, pkt_len);
}

void slirp_select_fill1(EthernetDevice *net, int *pfd_max, fd_set *rfds, fd_set *wfds, fd_set *efds, int *pdelay) {
    Slirp *slirp_state = net->opaque;
    slirp_select_fill(slirp_state, pfd_max, rfds, wfds, efds);
}

void slirp_select_poll1(EthernetDevice *net, fd_set *rfds, fd_set *wfds, fd_set *efds, int select_ret) {
    Slirp *slirp_state = net->opaque;
    slirp_select_poll(slirp_state, rfds, wfds, efds, (select_ret <= 0));
}


EthernetDevice *slirp_open(void) {
    EthernetDevice *net;
    struct in_addr  net_addr   = {.s_addr = htonl(0x0a000200)}; /* 10.0.2.0 */
    struct in_addr  mask       = {.s_addr = htonl(0xffffff00)}; /* 255.255.255.0 */
    struct in_addr  host       = {.s_addr = htonl(0x0a000202)}; /* 10.0.2.2 */
    struct in_addr  dhcp       = {.s_addr = htonl(0x0a00020f)}; /* 10.0.2.15 */
    struct in_addr  dns        = {.s_addr = htonl(0x0a000203)}; /* 10.0.2.3 */
    const char *    bootfile   = NULL;
    const char *    vhostname  = NULL;
    int             restricted = 0;

    if (slirp_state) {
        fprintf(dromajo_stderr, "Only a single slirp instance is allowed\n");
        return NULL;
    }
    net = mallocz(sizeof(*net));

    slirp_state = slirp_init(restricted, net_addr, mask, host, vhostname, "", bootfile, dhcp, dns, net);

    net->mac_addr[0]  = 0x02;
    net->mac_addr[1]  = 0x00;
    net->mac_addr[2]  = 0x00;
    net->mac_addr[3]  = 0x00;
    net->mac_addr[4]  = 0x00;
    net->mac_addr[5]  = 0x01;
    net->opaque       = slirp_state;
    net->write_packet = slirp_write_packet;
    net->select_fill  = slirp_select_fill1;
    net->select_poll  = slirp_select_poll1;

    return net;
}

#endif /* CONFIG_SLIRP */

