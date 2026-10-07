// HAL Network Abstraction
// Unified interface for network communication and services

#ifndef _HAL_NETWORK_H_
#define _HAL_NETWORK_H_

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// Network interface configuration
typedef struct {
    char ifname[16];             // Interface name (eth0, wlan0, etc.)
    bool dhcp_enable;            // Enable DHCP
    char ip_addr[16];            // IP address (if static)
    char netmask[16];            // Netmask (if static)
    char gateway[16];            // Gateway (if static)
    char dns1[16];               // Primary DNS server
    char dns2[16];               // Secondary DNS server
    char mac_addr[18];           // MAC address
    uint32_t mtu;                // MTU size
    bool promiscuous;            // Promiscuous mode
    void *priv;                  // Platform private data
} hal_netif_config_t;

// Wireless network configuration
typedef struct {
    char ssid[32];               // SSID
    char password[64];           // Password
    uint32_t security;           // Security mode: 0=open, 1=WEP, 2=WPA_PSK, 3=WPA2_PSK
    uint32_t channel;            // Channel number
    uint32_t mode;               // Mode: 0=infrastructure, 1=adhoc, 2=ap
    int32_t rssi;                // Signal strength (dBm)
    uint32_t rate;               // Data rate (Mbps)
    char bssid[18];              // BSSID (MAC address)
} hal_wifi_config_t;

// Network service configuration
typedef struct {
    bool http_enable;            // Enable HTTP server
    uint16_t http_port;          // HTTP port
    bool https_enable;           // Enable HTTPS server
    uint16_t https_port;         // HTTPS port
    bool rtsp_enable;            // Enable RTSP server
    uint16_t rtsp_port;          // RTSP port
    bool onvif_enable;           // Enable ONVIF server
    uint16_t onvif_port;         // ONVIF port
    bool ssh_enable;             // Enable SSH server
    uint16_t ssh_port;           // SSH port
    bool telnet_enable;          // Enable Telnet server
    uint16_t telnet_port;        // Telnet port
    bool ftp_enable;             // Enable FTP server
    uint16_t ftp_port;           // FTP port
    bool ntp_enable;             // Enable NTP client
    char ntp_server[64];         // NTP server address
    bool ddns_enable;            // Enable DDNS
    char ddns_provider[32];      // DDNS provider
    char ddns_domain[64];        // DDNS domain
    bool p2p_enable;             // Enable P2P service
    char p2p_server[64];         // P2P server address
    void *priv;                  // Platform private data
} hal_network_service_config_t;

// Network status
typedef struct {
    char ip_addr[16];            // Current IP address
    char netmask[16];            // Current netmask
    char gateway[16];            // Current gateway
    char mac_addr[18];           // MAC address
    uint32_t rx_bytes;           // Received bytes
    uint32_t tx_bytes;           // Transmitted bytes
    uint32_t rx_packets;         // Received packets
    uint32_t tx_packets;         // Transmitted packets
    uint32_t rx_errors;          // Receive errors
    uint32_t tx_errors;          // Transmit errors
    uint32_t link_speed;         // Link speed (Mbps)
    bool link_up;                // Link status
    uint32_t signal_strength;    // Signal strength (0-100 for wireless)
} hal_network_status_t;

// Network initialization
int hal_network_init(void);
int hal_network_deinit(void);

// Interface management
int hal_netif_up(const char *ifname);
int hal_netif_down(const char *ifname);
int hal_netif_set_config(const char *ifname, const hal_netif_config_t *config);
int hal_netif_get_config(const char *ifname, hal_netif_config_t *config);
int hal_netif_get_status(const char *ifname, hal_network_status_t *status);
int hal_netif_reset_stats(const char *ifname);

// Wireless network
int hal_wifi_init(void);
int hal_wifi_deinit(void);
int hal_wifi_scan(hal_wifi_config_t *networks, uint32_t max_count, uint32_t *count);
int hal_wifi_connect(const hal_wifi_config_t *config);
int hal_wifi_disconnect(void);
int hal_wifi_get_status(hal_wifi_config_t *config, hal_network_status_t *status);
int hal_wifi_start_ap(const hal_wifi_config_t *config);
int hal_wifi_stop_ap(void);

// Network services
int hal_network_start_services(const hal_network_service_config_t *config);
int hal_network_stop_services(void);
int hal_network_restart_services(void);
int hal_network_get_service_status(hal_network_service_config_t *config);

// HTTP/HTTPS server
int hal_http_start(uint16_t port, const char *web_root);
int hal_http_stop(void);
int hal_https_start(uint16_t port, const char *cert_path, const char *key_path);
int hal_https_stop(void);
int hal_http_set_auth(const char *username, const char *password);
int hal_http_add_cgi(const char *path, void (*handler)(void));

// RTSP server
int hal_rtsp_start(uint16_t port);
int hal_rtsp_stop(void);
int hal_rtsp_add_stream(uint32_t stream_id, const char *name, uint32_t venc_chn_id, uint32_t aenc_chn_id);
int hal_rtsp_remove_stream(uint32_t stream_id);
int hal_rtsp_set_auth(const char *username, const char *password);

// ONVIF server
int hal_onvif_start(uint16_t port);
int hal_onvif_stop(void);
int hal_onvif_set_device_info(const char *manufacturer, const char *model, const char *serial_number);
int hal_onvif_add_profile(uint32_t profile_id, const char *name, uint32_t width, uint32_t height, uint32_t fps);

// P2P service
int hal_p2p_start(const char *server_addr);
int hal_p2p_stop(void);
int hal_p2p_register(const char *device_id);
int hal_p2p_unregister(void);
int hal_p2p_get_connection_status(bool *connected);

// DDNS service
int hal_ddns_start(const char *provider, const char *domain, const char *username, const char *password);
int hal_ddns_stop(void);
int hal_ddns_update(void);
int hal_ddns_get_status(char *current_ip, size_t ip_len);

// NTP client
int hal_ntp_start(const char *server);
int hal_ntp_stop(void);
int hal_ntp_sync(void);
int hal_ntp_get_time(uint64_t *timestamp);

// QoS and traffic control
int hal_qos_set_priority(uint32_t class, uint32_t priority);
int hal_qos_set_bandwidth(uint32_t class, uint32_t min_rate, uint32_t max_rate);
int hal_qos_add_rule(uint32_t class, const char *src_ip, uint16_t src_port, 
                     const char *dst_ip, uint16_t dst_port, uint8_t protocol);

// Network security
int hal_firewall_add_rule(const char *src_ip, const char *dst_ip, uint16_t port, uint8_t protocol, bool allow);
int hal_firewall_remove_rule(uint32_t rule_id);
int hal_firewall_enable(bool enable);
int hal_firewall_get_status(bool *enabled, uint32_t *rule_count);

// VPN support
int hal_vpn_connect(const char *server, const char *username, const char *password);
int hal_vpn_disconnect(void);
int hal_vpn_get_status(bool *connected, char *server_ip, size_t ip_len);

// Network diagnostics
int hal_network_ping(const char *host, uint32_t timeout_ms, uint32_t *rtt_ms);
int hal_network_traceroute(const char *host, char *path, size_t path_len);
int hal_network_dns_lookup(const char *hostname, char *ip_addr, size_t ip_len);
int hal_network_get_route_table(char *table, size_t table_len);

// Socket abstraction (for platform-specific socket implementations)
typedef void* hal_socket_t;

hal_socket_t hal_socket_create(int domain, int type, int protocol);
int hal_socket_bind(hal_socket_t socket, const char *ip, uint16_t port);
int hal_socket_listen(hal_socket_t socket, int backlog);
hal_socket_t hal_socket_accept(hal_socket_t socket, char *client_ip, size_t ip_len, uint16_t *client_port);
int hal_socket_connect(hal_socket_t socket, const char *ip, uint16_t port);
int hal_socket_send(hal_socket_t socket, const void *data, size_t len);
int hal_socket_recv(hal_socket_t socket, void *data, size_t len, int timeout_ms);
int hal_socket_close(hal_socket_t socket);
int hal_socket_set_option(hal_socket_t socket, int level, int optname, const void *optval, socklen_t optlen);

// Multicast support
int hal_multicast_join(const char *group_ip, uint16_t port, const char *ifname);
int hal_multicast_leave(const char *group_ip, uint16_t port, const char *ifname);
int hal_multicast_send(const char *group_ip, uint16_t port, const void *data, size_t len);

// UPnP support
int hal_upnp_start(void);
int hal_upnp_stop(void);
int hal_upnp_add_port_mapping(uint16_t external_port, uint16_t internal_port, 
                              const char *protocol, const char *description);
int hal_upnp_remove_port_mapping(uint16_t external_port, const char *protocol);

// Network event callbacks
typedef void (*hal_network_event_cb_t)(int event_type, void *data, size_t data_len, void *user_data);

typedef enum {
    HAL_NET_EVENT_CONNECTED = 0,
    HAL_NET_EVENT_DISCONNECTED,
    HAL_NET_EVENT_IP_CHANGED,
    HAL_NET_EVENT_LINK_UP,
    HAL_NET_EVENT_LINK_DOWN,
    HAL_NET_EVENT_DHCP_SUCCESS,
    HAL_NET_EVENT_DHCP_FAILED,
    HAL_NET_EVENT_WIFI_SCAN_COMPLETE,
    HAL_NET_EVENT_WIFI_CONNECTED,
    HAL_NET_EVENT_WIFI_DISCONNECTED,
} hal_network_event_t;

int hal_network_register_callback(hal_network_event_t event, hal_network_event_cb_t callback, void *user_data);
int hal_network_unregister_callback(hal_network_event_t event, hal_network_event_cb_t callback);

// Platform-specific network operations
int hal_network_platform_init(void);
int hal_network_platform_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // _HAL_NETWORK_H_