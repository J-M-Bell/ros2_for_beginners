#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/int64.hpp"

using namespace std::placeholders;
using namespace std::chrono_literals;

/**
 * A node that creates a publisher for the 'number' topic a number
 * consistently.
 */
class NumberPublisherNode : public rclcpp::Node
{
public:
    NumberPublisherNode() : Node("number_publisher")
    {
        number_publisher_ = this->create_publisher<example_interfaces::msg::Int64>("number", 10);
        timer_ = this->create_wall_timer(0.5s, std::bind(&NumberPublisherNode::publishNumbers, this));

    }
private:
    /**
     * A callback function that prints a number to the 'number' topic.
     */
    void publishNumbers()
    {
        auto msgNumber = example_interfaces::msg::Int64();
        msgNumber.data = 2;
        number_publisher_->publish(msgNumber);

    }
    
    // node instance variables
    rclcpp::Publisher<example_interfaces::msg::Int64>::SharedPtr number_publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NumberPublisherNode>(); 
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}