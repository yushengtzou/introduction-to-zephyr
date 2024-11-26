#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/wifi_mgmt.h>

// Event callbacks
static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

// Semaphores
static K_SEM_DEFINE(sem_wifi_connected, 0, 1);
static K_SEM_DEFINE(sem_ipv4_obtained, 0, 1);

// Called when the WiFi is connected
static void handle_wifi_connected(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status) {
        printk("Error (%d): Connection request failed\r\n", status->status);
    } else {
        printk("Connected\r\n");
        k_sem_give(&sem_wifi_connected);
    }
}

// Called when the WiFi is disconnected
static void handle_wifi_disconnected(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status) {
        printk("Error (%d): Disconnection request failed\r\n", status->status);
    } else {
        printk("Disconnected\r\n");
        k_sem_take(&sem_wifi_connected, K_NO_WAIT);
    }
}

// Called when the IPv4 address is obtained from the DHCP server
static void handle_ipv4_obtained(struct net_if *iface)
{
    int i = 0;

    for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {

        char buf[NET_IPV4_ADDR_LEN];

        // Print the IPv4 address information
        printk("IPv4 address obtained:\r\n");
        printk("  IP address: %s\r\n",
                net_addr_ntop(AF_INET,
                              &iface->config.ip.ipv4->unicast[i].ipv4,
                              buf, sizeof(buf)));
        printk("  Subnet: %s\r\n",
                net_addr_ntop(AF_INET,
                              &iface->config.ip.ipv4->unicast[i].netmask,
                              buf, sizeof(buf)));
        printk("  Router: %s\r\n",
                net_addr_ntop(AF_INET,
                              &iface->config.ip.ipv4->gw,
                              buf, sizeof(buf)));
    }

    // Signal that the IP address has been obtained
    k_sem_give(&sem_ipv4_obtained);
}

// Event handler for WiFi management events
static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb, 
                                    uint32_t mgmt_event, 
                                    struct net_if *iface)
{
    switch (mgmt_event)
    {
        case NET_EVENT_WIFI_CONNECT_RESULT:
            handle_wifi_connected(cb);
            break;
        case NET_EVENT_WIFI_DISCONNECT_RESULT:
            handle_wifi_disconnected(cb);
            break;
        case NET_EVENT_IPV4_ADDR_ADD:
            handle_ipv4_obtained(iface);
            break;
        default:
            break;
    }
}

// Initialize the WiFi event callbacks
void wifi_init(void)
{
    // Initialize the event callbacks
    net_mgmt_init_event_callback(&wifi_cb, 
                                 wifi_mgmt_event_handler,
                                 NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_init_event_callback(&ipv4_cb, 
                                 wifi_mgmt_event_handler,
                                 NET_EVENT_IPV4_ADDR_ADD);

    // Add the event callbacks
    net_mgmt_add_event_callback(&wifi_cb);
    net_mgmt_add_event_callback(&ipv4_cb);
}

// Connect to the WiFi network (blocking)
int wifi_connect(char *ssid, char *psk)
{
    int ret;
    struct net_if *iface;
    struct wifi_connect_req_params params;

    // Get the default network interface
    iface = net_if_get_default();

    // Fill in the connect request parameters
    params.ssid = (const uint8_t *)ssid;
    params.ssid_length = strlen(ssid);
    params.psk = (const uint8_t *)psk;
    params.psk_length = strlen(psk);
    params.security = WIFI_SECURITY_TYPE_PSK;
    params.band = WIFI_FREQ_BAND_UNKNOWN;
	params.channel = WIFI_CHANNEL_ANY;
	params.mfp = WIFI_MFP_OPTIONAL;

    // Connect to the WiFi network
    ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, 
                   iface, 
                   &params, 
                   sizeof(params));
    
    // Wait for the connection to complete
    k_sem_take(&sem_wifi_connected, K_FOREVER);

    return ret;
}

// Get the WiFi status (blocking)
void wifi_wait_for_ip_addr(void)
{
    struct wifi_iface_status status;
    struct net_if *iface;

    // Get interface
    iface = net_if_get_default();

    // Wait for the IPv4 address to be obtained
    k_sem_take(&sem_ipv4_obtained, K_FOREVER);

    // Get the WiFi status
    if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, 
                 iface, 
                 &status,
                 sizeof(struct wifi_iface_status)))
    {
        printk("Error: WiFi status request failed\r\n");
    }

    // Print the WiFi status
    printk("WiFi status:\r\n");
    if (status.state >= WIFI_STATE_ASSOCIATED)
    {
        printk("  SSID: %-32s\r\n", status.ssid);
        printk("  Band: %s\r\n", wifi_band_txt(status.band));
        printk("  Channel: %d\r\n", status.channel);
        printk("  Security: %s\r\n", wifi_security_txt(status.security));
        printk("  RSSI: %d\r\n", status.rssi);
    }
}

// Disconnect from the WiFi network
void wifi_disconnect(void)
{
    struct net_if *iface = net_if_get_default();

    if (net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0))
    {
        printk("Error: Disconnection request failed\r\n");
    }
}
