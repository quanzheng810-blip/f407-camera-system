#include "network_service.h"

#include <stdio.h>

#include "bsp_eth.h"
#include "ethernetif.h"
#include "lwip/dhcp.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "rvm_timebase.h"
#include "ui_service.h"

#define RVM_NETWORK_POLL_PERIOD_MS  500U

static struct netif s_netif;
static NetworkState s_state = NETWORK_STATE_DISABLED;
static uint32_t s_last_poll_ms;

static void RVM_NetworkService_UpdateStatus(void)
{
    char text[64];

    if (!netif_is_link_up(&s_netif))
    {
        s_state = NETWORK_STATE_CONNECTING;
        RVM_UIService_SetNetworkStatus("Network\nlink down");
    }
    else if (ip4_addr_isany_val(*netif_ip4_addr(&s_netif)))
    {
        s_state = NETWORK_STATE_CONNECTING;
        RVM_UIService_SetNetworkStatus("Network\nDHCP...");
    }
    else
    {
        char address[16];

        s_state = NETWORK_STATE_CONNECTED;
        (void)ip4addr_ntoa_r(netif_ip4_addr(&s_netif),
                             address,
                             sizeof(address));
        (void)sprintf(text, "Network\n%s", address);
        RVM_UIService_SetNetworkStatus(text);
    }
}

bool RVM_NetworkService_Init(void)
{
    ip4_addr_t any_address;

    s_state = NETWORK_STATE_INIT;
    RVM_UIService_SetNetworkStatus("Network\ninitializing");
    if (!RVM_ETH_Init())
    {
        s_state = NETWORK_STATE_BACKOFF;
        RVM_UIService_SetNetworkStatus("Network\nPHY init failed");
        return false;
    }

    tcpip_init(NULL, NULL);
    ip4_addr_set_zero(&any_address);
    if (netifapi_netif_add(&s_netif,
                           &any_address,
                           &any_address,
                           &any_address,
                           NULL,
                           ethernetif_init,
                           tcpip_input) != ERR_OK)
    {
        s_state = NETWORK_STATE_BACKOFF;
        RVM_UIService_SetNetworkStatus("Network\nnetif failed");
        return false;
    }

    (void)netifapi_netif_set_default(&s_netif);
    (void)netifapi_netif_set_up(&s_netif);
    if (RVM_ETH_IsLinkUp())
    {
        (void)netifapi_netif_set_link_up(&s_netif);
    }
    else
    {
        (void)netifapi_netif_set_link_down(&s_netif);
    }
    (void)netifapi_dhcp_start(&s_netif);

    s_state = NETWORK_STATE_CONNECTING;
    s_last_poll_ms = 0U;
    RVM_NetworkService_UpdateStatus();
    return true;
}

void RVM_NetworkService_Process(void)
{
    bool physical_link;
    uint32_t now_ms;

    if ((s_state == NETWORK_STATE_DISABLED) ||
        (s_state == NETWORK_STATE_INIT) ||
        (s_state == NETWORK_STATE_BACKOFF))
    {
        return;
    }

    now_ms = RVM_Timebase_GetMilliseconds();
    if ((uint32_t)(now_ms - s_last_poll_ms) < RVM_NETWORK_POLL_PERIOD_MS)
    {
        return;
    }
    s_last_poll_ms = now_ms;

    physical_link = RVM_ETH_IsLinkUp();
    if (physical_link && !netif_is_link_up(&s_netif))
    {
        (void)netifapi_netif_set_link_up(&s_netif);
        (void)netifapi_dhcp_start(&s_netif);
    }
    else if (!physical_link && netif_is_link_up(&s_netif))
    {
        (void)netifapi_netif_set_link_down(&s_netif);
    }
    RVM_NetworkService_UpdateStatus();
}

NetworkState RVM_NetworkService_GetState(void)
{
    return s_state;
}
