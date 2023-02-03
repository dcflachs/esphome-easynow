#include "peer_base.h"
#include "esphome/core/log.h"

#define SEND_ACK_TIMEOUT_MS 2000

namespace esphome
{
  namespace proxy_base
  {
    std::vector<PeerBase *> *PeerBase::global_peer_list_ = new std::vector<PeerBase *>();

    //
    // Add ESPNow peer
    //

    bool PeerBase::add_espnow_peer(int espnow_channel)
    {
      ESP_LOGD(TAG->get_tag(), "Add peer %s", mac_address.as_string);

      memcpy(&peer_info_.peer_addr, &mac_address.as_uint8_t_array, sizeof(peer_info_.peer_addr));

      // peer_info_.peer_addr = mac_address.as_uint8_t_array;
      peer_info_.channel = espnow_channel;
      peer_info_.encrypt = false;

      if (esp_now_add_peer(&peer_info_) != ESP_OK)
      {
        ESP_LOGD(TAG->get_tag(), "Failed to add peer %s %s", name, mac_address.as_string);
        return false;
        ESP_LOGD(TAG->get_tag(), "Peer Added %s %s", name, mac_address.as_string);
      }
      global_peer_list_->push_back(this);

      last_state_change_millis_ = millis();
      reset_state("ESP Now Peer Added");

      return true;
    }

    //
    // Send Proxy Message
    //
    void PeerBase::send_proxy_message(proxy_message *message)
    {
      message->time_stamp = millis();
      proxy_message *queue_message = (proxy_message *)malloc(sizeof(proxy_message));

      memcpy(queue_message, message, sizeof(proxy_message));
      proxy_message_outgoing_queue_->push(queue_message);
    }

    //
    // Static callbacks (will find correct instance and call non static flavour)
    //
    PeerBase *PeerBase::find_peer_in_global_peer_list(PeerMacAddress *peer)
    {
      for (int i = 0; i < global_peer_list_->size(); i++)
      {
        if (global_peer_list_->at(i)->mac_address.mac_address_equals(peer))
          return global_peer_list_->at(i);
      }

      return NULL;
    }

    void PeerBase::call_on_data_send_callback(const uint8_t *mac_addr, esp_now_send_status_t status)
    {
      PeerMacAddress callback_peer;
      callback_peer.set_from_uint8_t_array(mac_addr);

      PeerBase *peer = find_peer_in_global_peer_list(&callback_peer);
      if (peer == NULL)
      {
        ESP_LOGD("PeerBaseComponent", "Received unexpected DataSent callback from unknown peer %s", callback_peer.as_string);
        return;
      }
      peer->on_data_send_callback(status);
    }

    void PeerBase::call_on_data_recv_callback(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
    {

      PeerMacAddress callback_peer;

      callback_peer.set_from_uint8_t_array(mac_addr);

      PeerBase *peer = find_peer_in_global_peer_list(&callback_peer);
      if (peer == NULL)
      {
        ESP_LOGD("PeerBaseComponent", "Received unexpected DataRecv callback from unknown peer %s", callback_peer.as_string);
        return;
      }
      peer->on_data_recv_callback(incomingData, len);
    }

    //
    // Non static callbacks
    //
    void PeerBase::on_data_send_callback(esp_now_send_status_t status)
    {
      if (!awaiting_send_ack_)
      {
        ESP_LOGD(TAG->get_tag(), "+ UNEXPECTED ACK %s - %s", mac_address.as_string, (status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"));
      }
      else
      {
        ESP_LOGD(TAG->get_tag(), "+ ACK %s - %s", mac_address.as_string, (status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"));
      }
      awaiting_send_ack_ = false;
    }

    void PeerBase::on_data_recv_callback(const uint8_t *incomingData, int len)
    {
      // Enqueue message as this is called from a high priority wifi thread, and documentation says not to spend too long in here...
      proxy_message *message = (proxy_message *)malloc(sizeof(proxy_message));

      memcpy(message, incomingData, sizeof(proxy_message));
      proxy_message_incoming_queue_->push(message);
    }

    //
    // Loop - this is called from the transmitter/receiver component
    //
    void PeerBase::loop()
    {
      if (process_proxy_message_incoming_queue())
      {
        return; // Dont do anything else as we are not supposed to block for long in loop
      }

      if (process_proxy_message_outgoing_queue())
      {
        return;
      }

      peer_workflow_loop();
    }

    //
    // Process message queues
    //
    bool PeerBase::process_proxy_message_incoming_queue()
    {
      if (proxy_message_incoming_queue_->size() == 0)
      {
        return false;
      }

      proxy_message *next_message = proxy_message_incoming_queue_->front();
      proxy_message_incoming_queue_->pop();

      std::string desc;
      describe_proxy_message(&desc, next_message);

      ESP_LOGD(TAG->get_tag(), "< %s %s", name, desc.c_str());

      handle_received_proxy_message(next_message);

      // And free it
      free(next_message);

      return true; // HAve processed message, so tell loop to not do other stuff
    }

    bool PeerBase::process_proxy_message_outgoing_queue()
    {
      // See if we have an outstanding send that we havent had an ACK for yet
      if (awaiting_send_ack_)
      {
        // See if we have times out awaiting the ack
        if ((millis() - awaiting_send_ack_start_ms_) > SEND_ACK_TIMEOUT_MS)
        {
          ESP_LOGD(TAG->get_tag(), "Timeout waiting for send ack after %dms", SEND_ACK_TIMEOUT_MS);
          awaiting_send_ack_ = false;
        }

        return true; // Dont allow loop to do other stuff as we are waiting for a send ACK or a timeout (or we have just got a timeout, so next time it will continue normally).
      }

      if (proxy_message_outgoing_queue_->size() == 0)
      {
        return false; // Nothing to send
      }

      proxy_message *next_message = proxy_message_outgoing_queue_->front();
      proxy_message_outgoing_queue_->pop();

      std::string desc;
      describe_proxy_message(&desc, next_message);

      esp_err_t result = esp_now_send(peer_info_.peer_addr, (uint8_t *)next_message, sizeof(proxy_message));
      awaiting_send_ack_ = true;
      awaiting_send_ack_start_ms_ = millis();

      if (result != ESP_OK)
      {
        const char *decoded_error = decode_espnow_error(result);
        ESP_LOGD(TAG->get_tag(), "> *FAILED* (%d) - %s -> %s %s", result, decoded_error, name, desc.c_str());
        return false;
      }
      ESP_LOGD(TAG->get_tag(), "> %s %s", name, desc.c_str());

      // And free it
      free(next_message);

      return true;
    }

    //
    // State
    //
    peer_state PeerBase::get_state()
    {
      return state_;
    }

    void PeerBase::set_state(peer_state state)
    {
      std::string old_desc;
      describe_peer_state(&old_desc, state_);

      state_ = state;

      std::string new_desc;
      describe_peer_state(&new_desc, state_);

      ESP_LOGD(TAG->get_tag(), "---> %s (was %s - after %f3s)", new_desc.c_str(), old_desc.c_str(), (millis() - last_state_change_millis_) / 1000.0);

      last_state_change_millis_ = millis();
    }

    void PeerBase::reset_state(const char *reason)
    {
      ESP_LOGD(TAG->get_tag(), "*** Resetting state - %s", reason);
      set_state(PS_READY);
    }

    //
    // Decode ESPNow errors
    //
    const char *PeerBase::decode_espnow_error(esp_err_t error)
    {
      switch (error)
      {
      case ESP_ERR_ESPNOW_BASE:
        return "ESP_ERR_ESPNOW_BASE";
      case ESP_ERR_ESPNOW_NOT_INIT:
        return "ESP_ERR_ESPNOW_NOT_INIT";
      case ESP_ERR_ESPNOW_ARG:
        return "ESP_ERR_ESPNOW_ARG";
      case ESP_ERR_ESPNOW_NO_MEM:
        return "ESP_ERR_ESPNOW_NO_MEM";
      case ESP_ERR_ESPNOW_FULL:
        return "ESP_ERR_ESPNOW_FULL";
      case ESP_ERR_ESPNOW_NOT_FOUND:
        return "ESP_ERR_ESPNOW_NOT_FOUND";
      case ESP_ERR_ESPNOW_INTERNAL:
        return "ESP_ERR_ESPNOW_INTERNAL";
      case ESP_ERR_ESPNOW_EXIST:
        return "ESP_ERR_ESPNOW_EXIST";
      case ESP_ERR_ESPNOW_IF:
        return "ESP_ERR_ESPNOW_IF";
      default:
        return "(UNKNOWN ERROR)";
      }
    }
  } // namespace proxy_base
} // namespace esphome
