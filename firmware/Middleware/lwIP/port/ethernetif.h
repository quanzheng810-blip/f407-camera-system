#ifndef RVM_ETHERNETIF_H
#define RVM_ETHERNETIF_H

#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);

#endif /* RVM_ETHERNETIF_H */
