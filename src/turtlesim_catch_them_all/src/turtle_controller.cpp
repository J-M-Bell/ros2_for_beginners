#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "my_robot_interfaces/msg/turtle.hpp"
#include "my_robot_interfaces/msg/turtle_array.hpp"
#include "my_robot_interfaces/srv/catch_turtle.hpp"
#include "turtlesim/srv/set_pen.hpp"
#include "turtlesim/srv/spawn.hpp"

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
        helper_turtle_sub_ = this->create_subscription<turtlesim::msg::Pose>(
            "/helper_turtle/pose", 10,
            std::bind(&TurtleControllerNode::set_cmd_helper_turtle_callback, this, _1));

        // Subscriber to get the array of alive turtles
        turtle_array_sub_ = this->create_subscription<my_robot_interfaces::msg::TurtleArray>(
            "/alive_turtles", 10, 
            std::bind(&TurtleControllerNode::alive_turtles_sub_callback, this, _1));


        // Publisher to send velocity commands to the 'main' turtle
        master_turtle_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
            "/turtle1/cmd_vel", 10);
        // Publisher to send velocity commands to the 'helper' turtle
        helper_turtle_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
            "/helper_turtle/cmd_vel", 10);

        // Client to call the catch turtle service
        catch_turtle_client_ = this->create_client<my_robot_interfaces::srv::CatchTurtle>("catch_turtle");

        // Store the first turtle's position
        first_turtle_.x = 5.544445;
        first_turtle_.y = 5.544445;

        catch_closest_turtle_first_ = this->declare_parameter("catch_closest_turtle_first", false);
    }

private:
    /**
     * A callback function that is responsible for publishing the current
     * pose to the desired pose of the 'main' turtle.
     * 
     * param msg The current pose of the 'main' turtle, received from the subscriber.
     */
    void set_cmd_callback(const turtlesim::msg::Pose::SharedPtr msg)
    {
        master_turtle_pose_ = *msg;

        if (catch_closest_turtle_first_ && !current_turtles_.turtles.empty()) {
            // Find the closest turtle in the array
            float min_distance = std::numeric_limits<float>::max();
            for (const auto& turtle_ : current_turtles_.turtles) {
                float dx = turtle_.x - master_turtle_pose_.x;
                float dy = turtle_.y - master_turtle_pose_.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                if (distance < min_distance) {
                    min_distance = distance;
                    first_turtle_ = turtle_;
                }
            }
        }
        
        /* Update the new pose to be the position of the first turtle in the array
        for beginning of launch*/
        for_master_turtle_pose_.x = first_turtle_.x;
        for_master_turtle_pose_.y = first_turtle_.y;

        // Calculate the distance and angle to the goal
        float delta_x = for_master_turtle_pose_.x - master_turtle_pose_.x;
        float delta_y = for_master_turtle_pose_.y - master_turtle_pose_.y;
        distance_ = std::sqrt(delta_x * delta_x + delta_y * delta_y);
        float current_theta = master_turtle_pose_.theta;
        float angle_to_goal = std::atan2(delta_y, delta_x);
        float angle_diff = angle_to_goal - current_theta;


        /* if block to check if the turtle is close enough to the new pose, 
         and if not, publish a cmd to move it closer */
        auto cmd_msg = geometry_msgs::msg::Twist();
        if (distance_ > 0.5)
        {
            cmd_msg.linear.x = kp * distance_; // Proportional control for linear velocity

            // Normalize the angle difference to the range [-pi, pi]
            if (angle_diff > M_PI) 
            {
                angle_diff -= 2 * M_PI;
            }
            else if (angle_diff < -M_PI) 
            {
                angle_diff += 2 * M_PI;
            }
            cmd_msg.angular.z = ka * angle_diff; // Proportional control for angular velocity
        }
        else // If close enough, stop the turtle and call the catch turtle service
        {
            // Stop the turtle
            cmd_msg.linear.x = 0.0;
            cmd_msg.angular.z = 0.0;

            // Call the catch turtle service
            call_catch_turtle(first_turtle_);
            
            // After calling the service, set a new random pen color for the turtle                                        
            call_set_pen();
        }
        master_turtle_pub_->publish(cmd_msg);
    }
    /**
     * A callback function that is responsible for publishing the current
     * pose to the desired pose of the 'helper' turtle.
     * 
     * param msg The current pose of the 'helper' turtle, received from the subscriber.
     */
    void set_cmd_helper_turtle_callback(const turtlesim::msg::Pose::SharedPtr msg)
    {
        if (current_turtles_.turtles.size() >= 5)
        {
            helper_turtle_pose_ = *msg;
            last_turtle_ = current_turtles_.turtles.back();
            
            /* Update the new pose to be the position of the first turtle in the array
            for beginning of launch*/
            for_helper_turtle_pose_.x = last_turtle_.x;
            for_helper_turtle_pose_.y = last_turtle_.y;

            // Calculate the distance and angle to the goal
            float delta_x = for_helper_turtle_pose_.x - helper_turtle_pose_.x;
            float delta_y = for_helper_turtle_pose_.y - helper_turtle_pose_.y;
            distance_ = std::sqrt(delta_x * delta_x + delta_y * delta_y);
            float current_theta = helper_turtle_pose_.theta;
            float angle_to_goal = std::atan2(delta_y, delta_x);
            float angle_diff = angle_to_goal - current_theta;


            /* if block to check if the turtle is close enough to the new pose, 
            and if not, publish a cmd to move it closer */
            auto cmd_msg = geometry_msgs::msg::Twist();
            if (distance_ > 0.5)
            {
                cmd_msg.linear.x = kp * distance_; // Proportional control for linear velocity

                // Normalize the angle difference to the range [-pi, pi]
                if (angle_diff > M_PI) 
                {
                    angle_diff -= 2 * M_PI;
                }
                else if (angle_diff < -M_PI) 
                {
                    angle_diff += 2 * M_PI;
                }
                cmd_msg.angular.z = ka * angle_diff; // Proportional control for angular velocity
            }
            else // If close enough, stop the turtle and call the catch turtle service
            {
                // Stop the turtle
                cmd_msg.linear.x = 0.0;
                cmd_msg.angular.z = 0.0;

                // Call the catch turtle service
                call_catch_turtle(last_turtle_);
            }
            helper_turtle_pub_->publish(cmd_msg);
        }
    }
    /**
     * A callback function that is responsible for storing the current
     * pose of the helper turtle.
     * 
     * param msg The current pose of the 'helper' turtle, received from the subscriber.
     */ 
    void set_helper_turtle_callback(const turtlesim::msg::Pose::SharedPtr msg)
    {
        // Store the current pose of the helper turtle
        helper_turtle_pose_ = *msg;
        // RCLCPP_INFO(this->get_logger(), "Helper turtle pose: (%.2f, %.2f)", msg->x, msg->y);
    }

    /**
     * A service call function that is responsible for calling the catch turtle service
     * when the 'main' turtle is close enough to the target turtle.
     * 
     * param caught_turtle_ The turtle that is being targeted to be caught, 
     *                      which is passed to the service request.
     */
    void call_catch_turtle(my_robot_interfaces::msg::Turtle caught_turtle_)
    {
        // Call the catch turtle service when close enough
        if (!catch_turtle_client_->wait_for_service(1s)) 
        {
            RCLCPP_ERROR(this->get_logger(), "Catch turtle service not available");
            return;
        }
        // Create a request with the name of the turtle to catch
        auto request = std::make_shared<my_robot_interfaces::srv::CatchTurtle::Request>();
        request->name = caught_turtle_.name;
        if (request->name == "helper_turtle") {
            return;
        }
        auto result_future = catch_turtle_client_->async_send_request(request, 
                                                std::bind(&TurtleControllerNode::catch_turtle_response_callback, this, _1));
    }
    /**
     * A service call function that is responsible for calling the set_pen service
     * to change the color of the 'main' turtle's pen after catching a turtle.
     */
    void call_set_pen()
    {
        auto set_pen_client = this->create_client<turtlesim::srv::SetPen>("turtle1/set_pen");
        if (!set_pen_client->wait_for_service(1s)) 
        {
            RCLCPP_ERROR(this->get_logger(), "Set pen service not available");
            return;
        }
        auto request = std::make_shared<turtlesim::srv::SetPen::Request>();
        request->r = static_cast<uint8_t>(rand() % 256);
        request->g = static_cast<uint8_t>(rand() % 256);
        request->b = static_cast<uint8_t>(rand() % 256);
        request->width = 3;
        request->off = false;
        set_pen_client->async_send_request(request);
    }

    /**
     * A service response callback function that is responsible for handling 
     * the response from the catch turtle service call.
     * 
     * param future The future object that contains the response from the 'catch turtle' service call, 
     *              which can be used to check if the turtle was successfully caught or not.
     */
    void catch_turtle_response_callback(rclcpp::Client<my_robot_interfaces::srv::CatchTurtle>::SharedFuture future)
    {
        auto response = future.get();
        // RCLCPP_INFO(this->get_logger(), "Service response: %s", response->message.c_str());
    }

    /**
     * A subscription callback function that is responsible for
     * subscribing to the array of alive turtles, 
     * and storing the first turtle in the array as a variable 
     * to be used for the 'main' turtle to chase.
     * 
     * param msg The array of alive turtles, received from the subscriber, 
     *           which contains the information of all the turtles that are 
     *           currently alive in the game. 
     */
    void alive_turtles_sub_callback(const my_robot_interfaces::msg::TurtleArray::SharedPtr msg)
    {
        current_turtles_ = *msg;
        if (!current_turtles_.turtles.empty()) { // If array is not empty, store the first turtle in the array as a variable
            first_turtle_ = current_turtles_.turtles[0];
            // RCLCPP_INFO(this->get_logger(), "First turtle in array: %s at (%.2f, %.2f)", 
            //             first_turtle_.name.c_str(), first_turtle_.x, first_turtle_.y);
        }
        // RCLCPP_INFO(this->get_logger(), "Size of turtle array: %zu", current_turtles_.turtles.size());
    }


    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr master_turtle_sub_;
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr helper_turtle_sub_;
    rclcpp::Subscription<my_robot_interfaces::msg::TurtleArray>::SharedPtr turtle_array_sub_;

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr master_turtle_pub_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr helper_turtle_pub_;

    rclcpp::Client<my_robot_interfaces::srv::CatchTurtle>::SharedPtr catch_turtle_client_;

    turtlesim::msg::Pose for_master_turtle_pose_;
    turtlesim::msg::Pose for_helper_turtle_pose_;
    turtlesim::msg::Pose master_turtle_pose_;
    turtlesim::msg::Pose helper_turtle_pose_;

    my_robot_interfaces::msg::TurtleArray current_turtles_;
    my_robot_interfaces::msg::Turtle first_turtle_;
    my_robot_interfaces::msg::Turtle last_turtle_;

    //pid control gains
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
