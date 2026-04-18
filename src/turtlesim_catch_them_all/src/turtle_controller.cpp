#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "my_robot_interfaces/msg/turtle.hpp"
#include "my_robot_interfaces/msg/turtle_array.hpp"
#include "my_robot_interfaces/srv/catch_turtle.hpp"

using namespace std::placeholders;
using namespace std::chrono_literals;

/**
 * A node that is respnsible for controlling the 'main'
 * turtle in the turtlesim game.
 */
class TurtleControllerNode : public rclcpp::Node
{
public:
    TurtleControllerNode() : Node("turtle_controller")
    {
        // Subscriber to get the current pose of the 'main' turtle
        master_turtle_sub_ = this->create_subscription<turtlesim::msg::Pose>(
            "/turtle1/pose", 10, 
            std::bind(&TurtleControllerNode::set_cmd_callback, this, _1));
        // Subscriber to get the array of alive turtles
        turtle_array_sub_ = this->create_subscription<my_robot_interfaces::msg::TurtleArray>(
            "/alive_turtles", 10, 
            std::bind(&TurtleControllerNode::turtleArrayCallback, this, _1));


        // Publisher to send velocity commands to the 'main' turtle
        master_turtle_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
            "/turtle1/cmd_vel", 10);
        
        // Client to call the catch turtle service
        catch_turtle_client_ = this->create_client<my_robot_interfaces::srv::CatchTurtle>("catch_turtle");

        // Store the first turtle's position
        first_turtle_.x = 5.544445;
        first_turtle_.y = 5.544445;

        catch_closest_turtle_first_ = this->declare_parameter("catch_closest_turtle_first", true);
    }

private:
    /**
     * A callback function that is responsible for publishing the current
     * pose to the desired pose of the 'main' turtle.
     */
    void set_cmd_callback(const turtlesim::msg::Pose::SharedPtr msg)
    {

        if (catch_closest_turtle_first_ && !current_turtles_.turtles.empty()) {
            // Find the closest turtle in the array
            float min_distance = std::numeric_limits<float>::max();
            for (const auto& turtle : current_turtles_.turtles) {
                float dx = turtle.x - msg->x;
                float dy = turtle.y - msg->y;
                float distance = std::sqrt(dx * dx + dy * dy);
                if (distance < min_distance) {
                    min_distance = distance;
                    first_turtle_ = turtle;
                }
            }
        }
        
        /* Update the new pose to be the position of the first turtle in the array
        for beginning of launch*/
        new_pose.x = first_turtle_.x;
        new_pose.y = first_turtle_.y;

        // Calculate the distance and angle to the goal
        float delta_x = new_pose.x - msg->x;
        float delta_y = new_pose.y - msg->y;
        distance_ = std::sqrt(delta_x * delta_x + delta_y * delta_y);
        float current_theta = msg->theta;
        float angle_to_goal = std::atan2(delta_y, delta_x);
        float angle_diff = angle_to_goal - current_theta;


        /* if block to check if the turtle is close enough to the new pose, 
         and if not, publish a cmd to move it closer */
        auto cmd_msg = geometry_msgs::msg::Twist();

        if (distance_ > 0.5)
        {
            cmd_msg.linear.x = kp * distance_;

            // Normalize the angle difference to the range [-pi, pi]
            if (angle_diff > M_PI) 
            {
                angle_diff -= 2 * M_PI;
            }
            else if (angle_diff < -M_PI) 
            {
                angle_diff += 2 * M_PI;
            }
            cmd_msg.angular.z = ka * angle_diff;
            master_turtle_pub_->publish(cmd_msg);
        }
        else
        {
            cmd_msg.linear.x = 0.0;
            cmd_msg.angular.z = 0.0;
                // Call the catch turtle service when close enough
            if (!catch_turtle_client_->wait_for_service(1s)) 
            {
                RCLCPP_ERROR(this->get_logger(), "Catch turtle service not available");
                return;
            }
            auto request = std::make_shared<my_robot_interfaces::srv::CatchTurtle::Request>();
            request->name = first_turtle_.name;
            auto result_future = catch_turtle_client_->async_send_request(request, std::bind(&TurtleControllerNode::catchTurtleResponseCallback, this, _1));
        }
        master_turtle_pub_->publish(cmd_msg);
    }

    void catchTurtleResponseCallback(rclcpp::Client<my_robot_interfaces::srv::CatchTurtle>::SharedFuture future)
    {
        auto response = future.get();
        // RCLCPP_INFO(this->get_logger(), "Service response: %s", response->message.c_str());
    }

    /**
     * A callback function that is responsible for subscribing to the array of alive turtles, 
     * and storing the first turtle in the array as a variable to be used for the 'main' turtle to chase.
     */
    void turtleArrayCallback(const my_robot_interfaces::msg::TurtleArray::SharedPtr msg)
    {
        current_turtles_ = *msg;
        if (!current_turtles_.turtles.empty()) {
            first_turtle_ = current_turtles_.turtles[0];
            // RCLCPP_INFO(this->get_logger(), "First turtle in array: %s at (%.2f, %.2f)", 
            //             first_turtle_.name.c_str(), first_turtle_.x, first_turtle_.y);
        }
        // RCLCPP_INFO(this->get_logger(), "Size of turtle array: %zu", current_turtles_.turtles.size());
    }



    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr master_turtle_sub_;
    rclcpp::Subscription<my_robot_interfaces::msg::TurtleArray>::SharedPtr turtle_array_sub_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr master_turtle_pub_;
    rclcpp::Client<my_robot_interfaces::srv::CatchTurtle>::SharedPtr catch_turtle_client_;
    turtlesim::msg::Pose new_pose;
    my_robot_interfaces::msg::TurtleArray current_turtles_;
    my_robot_interfaces::msg::Turtle first_turtle_;
    float kp = 2;
    float ka = 6;
    float distance_;
    bool catch_closest_turtle_first_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TurtleControllerNode>(); 
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
