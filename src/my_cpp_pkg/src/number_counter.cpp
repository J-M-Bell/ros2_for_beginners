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
    /**
     * A callback function that receives a number from the 'number' topic, 
     * adds to it the value of the counter multiplied by 2,
     * and publishes the result to the 'number_count' topic. 
     */
    void callbackNumbers(const example_interfaces::msg::Int64::SharedPtr msg)
    {
        auto msgNumber = example_interfaces::msg::Int64();
        newNumber_ = msg->data;
        msg->data = newNumber_ + (counter_ * 2);
        counter_++;
        msgNumber.data = msg->data;
        publisher_->publish(msgNumber);
    }

    /**
     * A callback function that resets the counter to 0 if the request data is true,
     * and sends a response message indicating whether the counter was reset or not.
     */
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

    // Member variables
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
