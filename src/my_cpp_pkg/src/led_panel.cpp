#include "rclcpp/rclcpp.hpp"
#include "my_robot_interfaces/msg/led_panel_state.hpp"
#include "my_robot_interfaces/srv/set_led.hpp"

using namespace std::placeholders;

/**
 * A node that publishes the led panel state to the 
 * 'led_panel_state' topic and provides a service to set individual LEDs.
 */
class LedPanelNode : public rclcpp::Node
{
public:
    LedPanelNode() : Node("led_panel")
    {
        this->declare_parameter("led_states", std::vector<int64_t>({0, 0, 0}));
        led_panel_ = this->get_parameter("led_states").as_integer_array();

        led_panel_publisher_ = this->create_publisher<my_robot_interfaces::msg::LedPanelState>("led_panel_state", 10);
        led_panel_server_ = this->create_service<my_robot_interfaces::srv::SetLed>("set_led", 
            std::bind(&LedPanelNode::led_panel_server_callback, this, _1, _2));
        timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&LedPanelNode::publishLedPanelStates, this));
    }

private:
    /**
     * A callback function that publishes the led panel states to the 'led_panel_state' topic.
     */
    void publishLedPanelStates()
    {
        auto states = my_robot_interfaces::msg::LedPanelState();
        states.led_panel_states = led_panel_;
        led_panel_publisher_->publish(states);
    }

    /**
     * A service callback function that updates the 'led_panel_' variable based on the
     * request data received from the 'set_led' service.
     */
    void led_panel_server_callback(const std::shared_ptr<my_robot_interfaces::srv::SetLed::Request> request,
                                   std::shared_ptr<my_robot_interfaces::srv::SetLed::Response> response)
    {
        int64_t led_number = request->led_number;
        int64_t led_state = request->led_state;

        if (led_number <= 0 || led_number >= static_cast<int64_t>(led_panel_.size()) || led_state < 0 || led_state > 1) {
            response->result = false;
            RCLCPP_WARN(this->get_logger(), "Incorrect input provided....");
            return;
        }
        
        led_panel_[led_number-1] = led_state;
        response->result = true;
    }

    // node instance variables
    rclcpp::Publisher<my_robot_interfaces::msg::LedPanelState>::SharedPtr led_panel_publisher_;
    rclcpp::Service<my_robot_interfaces::srv::SetLed>::SharedPtr led_panel_server_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::vector<int64_t> led_panel_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<LedPanelNode>(); // MODIFY NAME
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
