#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "my_robot_interfaces/msg/turtle.hpp"
#include "my_robot_interfaces/msg/turtle_array.hpp"

using namespace std::placeholders;
using namespace std::chrono_literals;

/**
 * A node that is responsible for controlling a 'running' turtle in the turtlesim game.
 * This node subscribes to the array of alive turtles, and if there are any turtles in
 * the array, it publishes random velocity commands to the cmd_vel topic of the first turtle in the array.
 * This creates a 'running' turtle that moves around the screen randomly, making it more challenging for the player to catch it.
 */
class RunningTurtleControllerNode : public rclcpp::Node 
{
public:
    RunningTurtleControllerNode() : Node("running_turtle_controller")
    {
        turtle_array_sub_ = this->create_subscription<my_robot_interfaces::msg::TurtleArray>(
            "/alive_turtles", 10, 
            std::bind(&RunningTurtleControllerNode::alive_turtles_sub_callback, this, _1));
        running_turtle_timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&RunningTurtleControllerNode::running_turtle_publish_callback, this));
    }

private:

    /**
     * This function is called every second by the timer. It checks if there are any turtles in the current_turtles_ array,
     * and if so, it publishes a random Twist message to the cmd_vel topic of the first turtle in the array.
     */
    void running_turtle_publish_callback()
    {
        if (!current_turtles_.turtles.empty()) 
        {
            auto cmd_msg = geometry_msgs::msg::Twist();
            std::string running_turtle_cmd_topic_name_ = "/" + first_turtle_.name + "/cmd_vel";
            RCLCPP_INFO(this->get_logger(), "Publishing to topic: %s", running_turtle_cmd_topic_name_.c_str());

            running_turtle_cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
                running_turtle_cmd_topic_name_, 10);

            cmd_msg.linear.x = static_cast<float_t>(rand() % 5);
            cmd_msg.angular.z = static_cast<float_t>(rand() % 5);

            running_turtle_cmd_pub_->publish(cmd_msg);
        }
    }

    /**
     * This function is called whenever a new TurtleArray message is received on the /alive_turtles topic. 
     * It updates the current_turtles_ variable with the new array of turtles, and if the array is not empty,
     * it stores the first turtle in the array as a variable for later use.
     * 
     * param msg The current array of alive turtles, received from the subscriber.
     */
    void alive_turtles_sub_callback(const my_robot_interfaces::msg::TurtleArray::SharedPtr msg)
    {
        current_turtles_ = *msg;
        if (!current_turtles_.turtles.empty()) { // If array is not empty, store the first turtle in the array as a variable
            first_turtle_ = current_turtles_.turtles[0];
            // RCLCPP_INFO(this->get_logger(), "First turtle in array: %s at (%.2f, %.2f)", 
            //             first_turtle_.name.c_str(), first_turtle_.x, first_turtle_.y);
        }
    }


    rclcpp::Subscription<my_robot_interfaces::msg::TurtleArray>::SharedPtr turtle_array_sub_;

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr running_turtle_cmd_pub_;
    
    rclcpp::TimerBase::SharedPtr running_turtle_timer_;

    my_robot_interfaces::msg::TurtleArray current_turtles_;
    my_robot_interfaces::msg::Turtle first_turtle_;

    turtlesim::msg::Pose first_turtle_pose_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RunningTurtleControllerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
