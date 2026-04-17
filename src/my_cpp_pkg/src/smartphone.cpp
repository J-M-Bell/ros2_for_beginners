#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/string.hpp"

using namespace std::placeholders;

/**
 * A node that prints a string and creates a subscriber to the 'robot_news' topic.
 */
class SmartphoneNode : public rclcpp::Node 
{
public:
    SmartphoneNode() : Node("smartphone")
    {
        subscriber_ = this->create_subscription<example_interfaces::msg::String>(
            "robot_news", 10, 
            std::bind(&SmartphoneNode::callbackRobotNews, this, _1));
        RCLCPP_INFO(this->get_logger(), "Smartphone has been started.");
    }

private:
    /**
     * A callback function that prints the message from the 'robot_news' topic
     * to the user.
     */
    void callbackRobotNews(const example_interfaces::msg::String::SharedPtr msg)
    {
        RCLCPP_INFO(this->get_logger(), "%s", msg->data.c_str());
    }

    // node instance variables
    rclcpp::Subscription<example_interfaces::msg::String>::SharedPtr subscriber_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SmartphoneNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
