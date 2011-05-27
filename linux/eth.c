/* Copyright 2011 Boston University. All rights reserved. */

/* Redistribution and use in source and binary forms, with or without modification, are */
/* permitted provided that the following conditions are met: */

/*    1. Redistributions of source code must retain the above copyright notice, this list of */
/*       conditions and the following disclaimer. */

/*    2. Redistributions in binary form must reproduce the above copyright notice, this list */
/*       of conditions and the following disclaimer in the documentation and/or other materials */
/*       provided with the distribution. */

/* THIS SOFTWARE IS PROVIDED BY BOSTON UNIVERSITY ``AS IS'' AND ANY EXPRESS OR IMPLIED */
/* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND */
/* FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BOSTON UNIVERSITY OR */
/* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR */
/* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR */
/* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF */
/* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/* The views and conclusions contained in the software and documentation are those of the */
/* authors and should not be interpreted as representing official policies, either expressed */
/* or implied, of Boston University */

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>
#include <netinet/ether.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../rawethlib/eth.h"

typedef struct net_handle_s {
  struct sockaddr_ll socket_address;
  int fd;
  unsigned char src_mac[ETH_ALEN];
} net_handle;

/* Shortens error checking */
void diep(char* s) {
  perror(s);
  exit(1);
}


/* 
 * Takes a net_handle to be filled in. The interface name we wish to send on
 * EX: "eth0". And the destination mac address in the form:
 * unsigned char dest_mac[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
 */
void 
initSend(net_handle* hnd, char *iface, unsigned char *dest_mac) {
  struct ifreq req;
  int i, ifindex;

  /*Create raw ethernet socket*/
  if((hnd->fd = socket(PF_PACKET, SOCK_RAW, htons(MY_ETH_PROTOCOL))) == -1)
    diep("socket()");

  /*Get interface index*/
  memset(&req,0,sizeof(req));
  strncpy(req.ifr_name,iface,IFNAMSIZ);
  if(ioctl(hnd->fd, SIOCGIFINDEX, &req) == -1)
    diep("SIOCGIFINDEX");
  ifindex = req.ifr_ifindex;

  /*Get MAC address*/
  if(ioctl(hnd->fd, SIOCGIFHWADDR, &req) == -1)
    diep("SIOCGIFHWADDR");
  for (i = 0; i < ETH_ALEN; i++)
    hnd->src_mac[i] = req.ifr_hwaddr.sa_data[i];

  memset(&(hnd->socket_address),0,sizeof(hnd->socket_address));
  hnd->socket_address.sll_family = AF_PACKET;
  hnd->socket_address.sll_ifindex = ifindex;
  hnd->socket_address.sll_halen = ETH_ALEN;
  for(i = 0; i < ETH_ALEN; i++) {
    hnd->socket_address.sll_addr[i] = dest_mac[i];
  }
}

/*
 * Takes a net_handle to be initialized and the name of the interface
 * we wish to receive on such as "eth0"
 */
void 
initRecv(net_handle* hnd, char* iface) {
  struct ifreq req;
  int i, ifindex, n;

  /*Create raw ethernet socket*/
  if((hnd->fd = socket(PF_PACKET, SOCK_RAW, htons(MY_ETH_PROTOCOL))) == -1)
    diep("socket()");

  /*Get interface index*/
  memset(&req,0,sizeof(req));
  strncpy(req.ifr_name,iface,IFNAMSIZ);
  if(ioctl(hnd->fd, SIOCGIFINDEX, &req) == -1)
    diep("SIOCGIFINDEX");
  ifindex = req.ifr_ifindex;

  /*Get MAC address*/
  if(ioctl(hnd->fd, SIOCGIFHWADDR, &req) == -1)
    diep("SIOCGIFHWADDR");
  for (i = 0; i < ETH_ALEN; i++)
    hnd->src_mac[i] = req.ifr_hwaddr.sa_data[i];

  memset(&(hnd->socket_address),0,sizeof(struct sockaddr_ll));
  hnd->socket_address.sll_family = AF_PACKET;
  hnd->socket_address.sll_protocol = htons(MY_ETH_PROTOCOL);
  hnd->socket_address.sll_ifindex = ifindex;

  if(bind(hnd->fd, (struct sockaddr *) &(hnd->socket_address),
	  sizeof(hnd->socket_address)) == -1)
    diep("bind()");
}

/*
 * Takes an already initialized net_handle and sends the passed in
 * payload
 */
void 
sendFrame(net_handle* hnd, void *data, int len) {
  void* buffer = (void*)calloc(ETH_FRAME_LEN,1);
  struct ethhdr *eh = (struct ethhdr *)buffer;

  //construct header
  memcpy(buffer, (void *)(hnd->socket_address.sll_addr), ETH_ALEN);
  memcpy(buffer+ETH_ALEN, (void *)hnd->src_mac, ETH_ALEN);
  eh->h_proto=htons(MY_ETH_PROTOCOL);

  //copy payload
  memcpy(buffer+(2*ETH_ALEN)+2,data,len);

  if(sendto(hnd->fd, buffer, ETH_FRAME_LEN, 0,
	    (struct sockaddr *) &(hnd->socket_address),
	    sizeof(hnd->socket_address)) == -1)
    diep("send()");
  free(buffer);
}

/*
 * Takes an already initialized and bound net_handle and populates
 * data with at most len bytes from the payload
 */
void 
recvFrame(net_handle* hnd, void *data, int len) {
  void* buffer = (void*)calloc(ETH_FRAME_LEN,1);
  struct ethhdr *eh = (struct ethhdr *)buffer;
  
  if(recvfrom(hnd->fd, buffer, ETH_FRAME_LEN, 0, NULL, NULL) == -1)
    diep("recvfrom()");

  //copy payload to user
  memcpy(data,buffer+(2*ETH_ALEN)+2,len);
  free(buffer);
}

void 
closeHandle(net_handle* hnd) {
  close(hnd->fd);
}

void
get_mac(char* mac, unsigned char* ret) {
  struct ether_addr* ea = ether_aton(mac);
  int i;
  for(i = 0; i < ETH_ALEN; i++)
    ret[i] = ea->ether_addr_octet[i];
}
  
