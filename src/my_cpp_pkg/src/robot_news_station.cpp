#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/string.hpp"

using namespace std::chrono_literals;

/**
 * A node that creates a publisher to the 'robot_news' topic and publishes a String message
 * containing the robot's name and a greeting every 0.5 seconds.
 */
class RobotNewsStationNode : public rclcpp::Node 
{
public:
    RobotNewsStationNode() : Node("robot_news_station")
    {
        publisher_ = this->create_publisher<example_interfaces::msg::String>("robot_news", 10);
        this->declare_parameter("robot_name", "R2D2");
        robot_name_ = this->get_parameter("robot_name").as_string();
        timer_ = this->create_wall_timer(0.5s, std::bind(&RobotNewsStationNode::publishNews, this));
        RCLCPP_INFO(this->get_logger(), "Robot News Station has been started");
    }

private:
    /**
     * A function that creates a String message containing the robot's name and a greeting,
     * and publishes it to the 'robot_news' topic.
     */
    void publishNews()
    {
        auto msg = example_interfaces::msg::String();
        msg.data = std::string("Hi, this is ") + robot_name_ + std::string(" from the robot news station.");
        publisher_->publish(msg);
    }

    // Member variables
    std::string robot_name_;
    rclcpp::Publisher<example_interfaces::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RobotNewsStationNode>(); 
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
