#include "wifi.h"
#include "wifi_internal.h"
#include "user_event.h"

#include <logging.h>

#include <esp_err.h>
#include <esp_event.h>

const char *tcpip_adapter_if_str(tcpip_adapter_if_t adapter_if) {
  switch (adapter_if) {
    case TCPIP_ADAPTER_IF_STA:  return "STA";
    case TCPIP_ADAPTER_IF_AP:   return "AP";
    case TCPIP_ADAPTER_IF_ETH:  return "ETH";
    case TCPIP_ADAPTER_IF_TEST: return "TEST";
    default:                    return "?";
  }
}

const char *wifi_err_reason_str(wifi_err_reason_t reason) {
  switch (reason) {
    case WIFI_REASON_UNSPECIFIED: return "UNSPECIFIED";
    case WIFI_REASON_AUTH_EXPIRE: return "AUTH_EXPIRE";
    case WIFI_REASON_AUTH_LEAVE: return "AUTH_LEAVE";
    case WIFI_REASON_ASSOC_EXPIRE: return "ASSOC_EXPIRE";
    case WIFI_REASON_ASSOC_TOOMANY: return "ASSOC_TOOMANY";
    case WIFI_REASON_NOT_AUTHED: return "NOT_AUTHED";
    case WIFI_REASON_NOT_ASSOCED: return "NOT_ASSOCED";
    case WIFI_REASON_ASSOC_LEAVE: return "ASSOC_LEAVE";
    case WIFI_REASON_ASSOC_NOT_AUTHED: return "ASSOC_NOT_AUTHED";
    case WIFI_REASON_DISASSOC_PWRCAP_BAD: return "DISASSOC_PWRCAP_BAD";
    case WIFI_REASON_DISASSOC_SUPCHAN_BAD: return "DISASSOC_SUPCHAN_BAD";
    case WIFI_REASON_IE_INVALID: return "IE_INVALID";
    case WIFI_REASON_MIC_FAILURE: return "MIC_FAILURE";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: return "4WAY_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT: return "GROUP_KEY_UPDATE_TIMEOUT";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS: return "IE_IN_4WAY_DIFFERS";
    case WIFI_REASON_GROUP_CIPHER_INVALID: return "GROUP_CIPHER_INVALID";
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID: return "PAIRWISE_CIPHER_INVALID";
    case WIFI_REASON_AKMP_INVALID: return "AKMP_INVALID";
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION: return "UNSUPP_RSN_IE_VERSION";
    case WIFI_REASON_INVALID_RSN_IE_CAP: return "INVALID_RSN_IE_CAP";
    case WIFI_REASON_802_1X_AUTH_FAILED: return "802_1X_AUTH_FAILED";
    case WIFI_REASON_CIPHER_SUITE_REJECTED: return "CIPHER_SUITE_REJECTED";

    case WIFI_REASON_INVALID_PMKID: return "INVALID_PMKID";

    case WIFI_REASON_BEACON_TIMEOUT: return "BEACON_TIMEOUT";
    case WIFI_REASON_NO_AP_FOUND: return "NO_AP_FOUND";
    case WIFI_REASON_AUTH_FAIL: return "AUTH_FAIL";
    case WIFI_REASON_ASSOC_FAIL: return "ASSOC_FAIL";
    case WIFI_REASON_HANDSHAKE_TIMEOUT: return "HANDSHAKE_TIMEOUT";
    case WIFI_REASON_CONNECTION_FAIL: return "CONNECTION_FAIL";
    case WIFI_REASON_AP_TSF_RESET: return "AP_TSF_RESET";
    case WIFI_REASON_BASIC_RATE_NOT_SUPPORT: return "BASIC_RATE_NOT_SUPPORT";

    default: return "?";
  }
}

static void on_wifi_ready()
{
  LOG_INFO(" ");
}

static void on_scan_done(wifi_event_sta_scan_done_t *event)
{
  LOG_INFO("status=%s number=%u", event->status ? "failure" : "success", event->number);
}

static void on_sta_start()
{
  LOG_INFO(" ");
}

static void on_sta_stop()
{
  LOG_INFO(" ");
}

static void on_sta_connected(wifi_event_sta_connected_t *event)
{
  LOG_INFO("ssid=%.32s bssid=%02x:%02x:%02x:%02x:%02x:%02x channel=%u authmode=%s",
    event->ssid,
    event->bssid[0], event->bssid[1], event->bssid[2], event->bssid[3], event->bssid[4], event->bssid[5],
    event->channel,
    wifi_auth_mode_str(event->authmode)
  );
}

static void on_sta_disconnected(wifi_event_sta_disconnected_t *event)
{
  LOG_INFO("ssid=%.32s bssid=%02x:%02x:%02x:%02x:%02x:%02x reason=%s",
    event->ssid,
    event->bssid[0], event->bssid[1], event->bssid[2], event->bssid[3], event->bssid[4], event->bssid[5],
    wifi_err_reason_str(event->reason)
  );

  user_event(USER_EVENT_DISCONNECTED);
}

static void on_sta_authmode_change(wifi_event_sta_authmode_change_t *event)
{
  LOG_INFO("old=%s new=%s", wifi_auth_mode_str(event->old_mode), wifi_auth_mode_str(event->new_mode));
}

static void on_sta_bss_rssi_low(wifi_event_bss_rssi_low_t *event)
{
  LOG_INFO("rssi=%d", event->rssi);
}

static void on_sta_got_ip(ip_event_got_ip_t *event)
{
  LOG_INFO("if=%s ip=%d.%d.%d.%d changed=%s",
    tcpip_adapter_if_str(event->if_index),
    ip4_addr1_val(event->ip_info.ip), ip4_addr2_val(event->ip_info.ip), ip4_addr3_val(event->ip_info.ip), ip4_addr4_val(event->ip_info.ip),
    event->ip_changed ? "true" : "false"
  );

  user_event(USER_EVENT_CONNECTED);
}

static void on_sta_lost_ip()
{
  LOG_INFO(" ");

  // XXX: ???
  user_event(USER_EVENT_DISCONNECTED);
}

static void tcpip_adapter_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  switch (event_id) {
    case IP_EVENT_STA_GOT_IP:
      return on_sta_got_ip(event_data);

    case IP_EVENT_STA_LOST_IP:
      return on_sta_lost_ip();
  }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  switch (event_id) {
    case WIFI_EVENT_WIFI_READY:
      return on_wifi_ready();

    case WIFI_EVENT_SCAN_DONE:
      return on_scan_done(event_data);

    case WIFI_EVENT_STA_START:
      return on_sta_start();

    case WIFI_EVENT_STA_STOP:
      return on_sta_stop();

    case WIFI_EVENT_STA_CONNECTED:
      return on_sta_connected(event_data);

    case WIFI_EVENT_STA_DISCONNECTED:
      return on_sta_disconnected(event_data);

    case WIFI_EVENT_STA_AUTHMODE_CHANGE:
      return on_sta_authmode_change(event_data);

    case WIFI_EVENT_STA_BSS_RSSI_LOW:
      return on_sta_bss_rssi_low(event_data);
  }
}

int init_wifi_events()
{
  esp_err_t err;

  if ((err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &tcpip_adapter_event_handler, NULL))) {
    LOG_ERROR("esp_event_handler_register WIFI_EVENT: %s", esp_err_to_name(err));
    return -1;
  }

  if ((err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL))) {
    LOG_ERROR("esp_event_handler_register WIFI_EVENT: %s", esp_err_to_name(err));
    return -1;
  }

  return 0;
}
