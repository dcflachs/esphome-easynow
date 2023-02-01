#include "peer_transmitter.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <WiFi.h>

namespace esphome
{
  namespace proxy_receiver
  {
    void PeerTransmitter::handle_received_proxy_message(proxy_base::proxy_message *message)
    {
      if (message->message_type == proxy_base::T_TO_R_CHECKIN)
      {
        // Send a response
        proxy_base::proxy_message msg;
        msg.message_type = proxy_base::R_TO_T_CHECKIN_RESP;
        send_proxy_message(&msg);
      }
    }

    void PeerTransmitter::loop()
    {
    }
  } // namespace proxy_receiver
} // namespace esphome
