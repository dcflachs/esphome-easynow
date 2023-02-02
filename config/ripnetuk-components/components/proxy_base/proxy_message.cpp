#include "proxy_messages.h"

namespace esphome
{
    namespace proxy_base
    {
        void describe_proxy_message(std::string *output, proxy_message *message)
        {
            switch (message->message_type)
            {
            case T_TO_R_CHECKIN:
                output->append("T to R Check in");
                break;
            case R_TO_T_CHECKIN_RESP:
                output->append("R to C Check in response");
                break;
            case T_TO_R_SEND_SENSOR_STATE:
            {
                std::string state = std::to_string(message->send_sensor_state.state);
                std::string proxy_id = std::string(message->send_sensor_state.proxy_id);
                output->append("T to R Send Sensor State (" + proxy_id + ") State - " + state);
                break;
            }
            case R_TO_T_SEND_SENDOR_STATE_REPONSE:
            {
                std::string proxy_id = std::string(message->send_sensor_state.proxy_id);
                output->append("R to C Send Sensor State Response (" + proxy_id + ")");
                break;
            }
            default:
                output->append("!! UNKONWN MESSAGE TYPE !!");
            }
        }
    } // namespace proxy_base
} // namespace esphome

// output->append("(PING)" );
