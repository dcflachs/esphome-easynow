#include "peer_receiver.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <WiFi.h>

#define RESPONSE_TIMEOUT 5000
#define READY_TO_CHECKIN_DELAY 5000
#define READ_SENSORS_TIMEOUT 2000

namespace esphome
{
  namespace proxy_transmitter
  {
    void PeerReceiver::handle_received_proxy_message(proxy_base::proxy_message *message)
    {
      if (get_state() == proxy_base::PS_T_AWAIT_R_TO_T_CHECKIN_RESP)
      {
        if (message->message_type == proxy_base::R_TO_T_CHECKIN_RESP)
        {
          start_sensor_reads();
          set_state(proxy_base::PS_T_READING_SENSORS);
        }
        return;
      }

      ESP_LOGD(TAG->get_tag(), "Unexpected message type %d when in state %d", message->message_type, get_state());
    }

    void PeerReceiver::loop()
    {
      int time_since_last_state_change_ms = millis() - last_state_change_millis_;

      if (get_state() == proxy_base::PS_READY)
      {
        if (time_since_last_state_change_ms > READY_TO_CHECKIN_DELAY)
        {
          // Want to send a checkin
          proxy_base::proxy_message msg;
          msg.message_type = proxy_base::T_TO_R_CHECKIN;
          send_proxy_message(&msg);
          // Set state to awaiting R_TO_T_CHECKIN_RESP
          set_state(proxy_base::PS_T_AWAIT_R_TO_T_CHECKIN_RESP);
        }
        return;
      }

      if (get_state() == proxy_base::PS_T_AWAIT_R_TO_T_CHECKIN_RESP)
      {
        if (time_since_last_state_change_ms > RESPONSE_TIMEOUT)
        {
          reset_state("Timeout waiting for R to T Check in respponse");
        }
        return;
      }

      if (get_state() == proxy_base::PS_T_READING_SENSORS)
      {
        if (time_since_last_state_change_ms > READ_SENSORS_TIMEOUT)
        {
          reset_state("Timeout reading sensors");
        }

        bool has_outstanding_reads = false;
        for (int i = 0; i < sensors->size(); i++)
        {
          if (!sensors->at(i)->get_has_state())
          {
            has_outstanding_reads = true;
          }
        }

        if (!has_outstanding_reads <= 0)
        {
          reset_state("Finished");
        }
        return;
      }

      ESP_LOGD(TAG->get_tag(), "Unexpected state in loop - %d ", get_state());
    }

    void PeerReceiver::start_sensor_reads()
    {
      ESP_LOGD(TAG->get_tag(), "Reading %d sensors ", sensors->size());
      for (int i = 0; i < sensors->size(); i++)
      {
        SensorHolder *sensor = sensors->at(i);
        sensor->update();
      }
    }

  } // namespace proxy_receiver
} // namespace esphome
