#include "rclcpp/rclcpp.hpp"
#include "my_robot_interfaces/srv/set_led.hpp"

using namespace std::chrono_literals;
using namespace std::placeholders;

/**
 * This node simulates a battery level by counting up to 10. 
 * When the counter reaches 4, the battery level is considered empty 
 * and the LED will be turned off.
 */
class BatteryNode : public rclcpp::Node
{
public:
    BatteryNode() : Node("battery")
    {
        set_led_client_ = this->create_client<my_robot_interfaces::srv::SetLed>("set_led");
        battery_level_counter = 0;
        battery_level_empty = false;
        timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&BatteryNode::counter_callback, this));
    }

private:
    /**
     * A timer callback that simulates the battery level by counting up to 10.
     * When the counter reaches 4, the battery level is considered empty and the LED will be turned off. 
     * When the counter reaches 10, the battery level is considered full and the LED will be turned on. 
     * Then the counter is reset to 0 to repeat the cycle.
     */
    void counter_callback()
    {
        battery_level_counter += 1;
        if (battery_level_counter == 4)
        {
            battery_level_empty = true;
            RCLCPP_INFO(this->get_logger(), "Battery level is empty");
            call_set_led(1, true); // Turn off LED when battery is empty
        }
        else if (battery_level_counter == 10)
        {
            battery_level_empty = false;
            RCLCPP_INFO(this->get_logger(), "Battery level is full");
            call_set_led(1, false); // Turn on LED when battery is full
            battery_level_counter = 0;
        }
    }

    /**
     * A service client callback that calls the SetLed service to turn on or off the LED depending on the battery level.
     */
    void call_set_led(int led_number, bool led_state)
    {
        while(!set_led_client_->wait_for_service(1.0s))
        {
            RCLCPP_WARN(this->get_logger(), "Waiting for the server...");
        }
        auto request = std::make_shared<my_robot_interfaces::srv::SetLed::Request>();
        request->led_number = led_number;
        request->led_state = led_state;

        set_led_client_->async_send_request(request);
    }
    
    rclcpp::Client<my_robot_interfaces::srv::SetLed>::SharedPtr set_led_client_;
    rclcpp::TimerBase::SharedPtr timer_;
    int battery_level_counter;
    bool battery_level_empty;
};


int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<BatteryNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
