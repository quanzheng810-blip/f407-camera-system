#ifndef RVM_LWIPOPTS_H
#define RVM_LWIPOPTS_H

#define NO_SYS                          0
#define SYS_LIGHTWEIGHT_PROT            1

#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        (16U * 1024U)
#define MEMP_NUM_PBUF                   12
#define MEMP_NUM_RAW_PCB                2
#define MEMP_NUM_UDP_PCB                4
#define MEMP_NUM_TCP_PCB                5
#define MEMP_NUM_TCP_PCB_LISTEN         2
#define MEMP_NUM_TCP_SEG                24
#define MEMP_NUM_SYS_TIMEOUT            12

#define PBUF_POOL_SIZE                  8
#define PBUF_POOL_BUFSIZE               1536

#define LWIP_IPV4                       1
#define LWIP_IPV6                       0
#define LWIP_ARP                        1
#define LWIP_ICMP                       1
#define LWIP_RAW                        1
#define LWIP_DHCP                       1
#define LWIP_AUTOIP                     0
#define LWIP_DNS                        1
#define LWIP_IGMP                       0

#define LWIP_TCP                        1
#define TCP_MSS                         1460
#define TCP_WND                         (4U * TCP_MSS)
#define TCP_SND_BUF                     (4U * TCP_MSS)
#define TCP_SND_QUEUELEN                12
#define TCP_QUEUE_OOSEQ                 0

#define LWIP_UDP                        1
#define LWIP_NETCONN                    1
#define LWIP_SOCKET                     1
#define LWIP_NETIF_API                  1
#define LWIP_SO_RCVTIMEO                1
#define LWIP_SO_SNDTIMEO                1

#define LWIP_NETIF_HOSTNAME             1
#define LWIP_NETIF_STATUS_CALLBACK      1
#define LWIP_NETIF_LINK_CALLBACK        1

#define LWIP_STATS                      0
#define LWIP_PROVIDE_ERRNO              1

#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_GEN_ICMP               1
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_UDP              1
#define CHECKSUM_CHECK_TCP              1
#define CHECKSUM_CHECK_ICMP             1

#define TCPIP_THREAD_NAME               "lwIP"
#define TCPIP_THREAD_STACKSIZE          768
#define TCPIP_THREAD_PRIO               5
#define TCPIP_MBOX_SIZE                 12

#define DEFAULT_THREAD_STACKSIZE        512
#define DEFAULT_THREAD_PRIO             3
#define DEFAULT_RAW_RECVMBOX_SIZE       6
#define DEFAULT_UDP_RECVMBOX_SIZE       6
#define DEFAULT_TCP_RECVMBOX_SIZE       8
#define DEFAULT_ACCEPTMBOX_SIZE         4

#endif /* RVM_LWIPOPTS_H */
