#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/int64.hpp"
#include "example_interfaces/srv/set_bool.hpp"

using namespace std::placeholders;
using namespace std::chrono_literals;

/**
 * A node that creates a subscriber to the 'number' topic and publishes a new number
 * to the 'number_count' topic.
 */
class NumberCounterNode : public rclcpp::Node
{
public:
    NumberCounterNode() : Node("number_counter")
    {
        subscriber_ = this->create_subscription<example_interfaces::msg::Int64>(
            "number", 10, 
            std::bind(&NumberCounterNode::callbackNumbers, this, _1));
        

        publisher_ = this->create_publisher<example_interfaces::msg::Int64>(
            "number_count", 10);

        server_ = this->create_service<example_interfaces::srv::SetBool>(
            "reset_counter",
            std::bind(&NumberCounterNode::callbackResetCounter, this, _1, _2));
    
        RCLCPP_INFO(this->get_logger(), "Number counter has been started.");

    }

private:
    void callbackNumbers(const example_interfaces::msg::Int64::SharedPtr msg)
    {
        auto msgNumber = example_interfaces::msg::Int64();
        newNumber_ = msg->data;
        msg->data = newNumber_ + (counter_ * 2);
        counter_++;
        msgNumber.data = msg->data;
        // publishNewNumber(msg);
        publisher_->publish(msgNumber);
    }

    void callbackResetCounter(const example_interfaces::srv::SetBool::Request::SharedPtr request,
                              const example_interfaces::srv::SetBool::Response::SharedPtr response)
    {
        if (request->data)
        {
            counter_ = 0;
            response->message = std::string("Counter has been reset.");
            response->success = true;
        }
        else
        {
            response->message = "Counter has not been reset.";
            response->success = false;
        }
        RCLCPP_INFO(this->get_logger(), "%s", response->message.c_str());
    }
    rclcpp::Subscription<example_interfaces::msg::Int64>::SharedPtr subscriber_;
    rclcpp::Publisher<example_interfaces::msg::Int64>::SharedPtr publisher_;
    rclcpp::Service<example_interfaces::srv::SetBool>::SharedPtr server_;
    int newNumber_;
    int counter_ = 1;

};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<NumberCounterNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
