#include "proxy_receiver.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "../proxy_base/proxy_payload.h"
#include <WiFi.h>

namespace esphome
{
  namespace proxy_receiver
  {

    static const char *TAG = "proxy_receiver";

    void ProxyReceiverComponent::loop()
    {
      proxy_base::ProxyPayload pl;
      ESP_LOGD(TAG, "%s Time %d magic %d", WiFi.macAddress().c_str(), millis(), pl.get_magic_number());
      // uint8_t mac[6];
      // esp_wifi_get_mac(WIFI_IF_STA, mac);
      // char mac_str[18];
      // sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      // ESP_LOGD("my_component", "MAC address: %s", mac_str);
    }

    float ProxyReceiverComponent::get_setup_priority() const
    {
      return setup_priority::LATE;
    }

  } // namespace proxy_receiver
} // namespace esphome
