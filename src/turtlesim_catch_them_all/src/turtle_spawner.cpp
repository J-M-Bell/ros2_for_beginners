#include "rclcpp/rclcpp.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "turtlesim/srv/kill.hpp"
#include "my_robot_interfaces/msg/turtle.hpp"
#include "my_robot_interfaces/msg/turtle_array.hpp"
#include "my_robot_interfaces/srv/catch_turtle.hpp"

using namespace std::placeholders;
using namespace std::chrono_literals;


/**
 * A node that is responsible for spawning new turtles in the turtlesim game.
 */
class TurtleSpawnerNode : public rclcpp::Node
{
public:
    TurtleSpawnerNode() : Node("turtle_spawner")
    {
        spawn_frequency_ = this->declare_parameter("spawn_frequency", 1);
        turtle_name_prefix_ = this->declare_parameter("turtle_name_prefix", "turtle");

        turtle_counter_ = 1;

        // Create a client and timerfor the spawn service
        spawn_client_ = this->create_client<turtlesim::srv::Spawn>("/spawn");
        spawn_timer_ = this->create_timer(std::chrono::duration<double>(1.0 / spawn_frequency_), 
                                        std::bind(&TurtleSpawnerNode::call_spawn, this));


        // Create a publisher for the array of alive turtles
        turtle_array_publisher_ = this->create_publisher<my_robot_interfaces::msg::TurtleArray>("/alive_turtles", 10);
        turtle_array_timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&TurtleSpawnerNode::alive_turtle_publish_callback, this));

        // Create a service to catch turtles
        catch_turtle_server_ = this->create_service<my_robot_interfaces::srv::CatchTurtle>(
            "catch_turtle",
            std::bind(&TurtleSpawnerNode::catch_turtle_server_callback, this, _1, _2)
        );

    }


private:
    /**
     * A timer callback that is responsible for calling the spawn service to spawn a new turtle at a random location every 5 seconds.
     */
    void call_spawn()
    {
        // Wait for the service to be available
        while (!spawn_client_->wait_for_service(std::chrono::seconds(1))) 
        {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
                return;
            }
            RCLCPP_INFO(this->get_logger(), "Service not available, waiting again...");
        }

        // Create a request to spawn a turtle at a random location
        auto request = std::make_shared<turtlesim::srv::Spawn::Request>();

        // Generate coordinates for the new turtle
        request->x = static_cast<float>(rand() % 11); // Random x between 0 and 10
        request->y = static_cast<float>(rand() % 11); // Random y between 0 and 10
        request->theta = 0.0; // Default orientation

        if (turtle_counter_ == 1) // Spawn a helper turtle at the beginning of the game
        {
            request->name = "helper_turtle";
        }
        else 
        {
            request->name = turtle_name_prefix_ + std::to_string(turtle_counter_);
        }
        turtle_counter_ += 1;

        // Store the new turtle's position to be added to array
        new_turtle_x = request->x;
        new_turtle_y = request->y;
        

        // Call the service to spawn the turtle
        auto result_future = spawn_client_->async_send_request(request, std::bind(&TurtleSpawnerNode::spawn_response_callback, this, _1));
        // RCLCPP_INFO(this->get_logger(), "Spawned turtle at (%.2f, %.2f)", request->x, request->y);
    }

    /**
     * A callback function that is called when the spawn service returns a response.
     * 
     * param future The future object that contains the response from the spawn service call, 
     *              which can be used to get the name of the newly spawned turtle and add it 
     *              to the list of alive turtles if it's not the helper turtle.
     */
    void spawn_response_callback(rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future)
    {
        auto response = future.get();
        // RCLCPP_INFO(this->get_logger(), "Spawned turtle %s", response->name.c_str());
        
        // Add the new turtle to the list of alive turtles if not the helper turtle
        new_turtle_name = response->name;
        if (response->name != "helper_turtle") 
        {
            my_robot_interfaces::msg::Turtle new_turtle;
            new_turtle.name = response->name;
            new_turtle.x = new_turtle_x;
            new_turtle.y = new_turtle_y;
            turtles_.push_back(new_turtle);
        }
    }

    /**
     * A timer callback that is responsible for publishing the array of alive turtles every second.
     */
    void alive_turtle_publish_callback()
    {
        auto turtle_array_msg = my_robot_interfaces::msg::TurtleArray();
        turtle_array_msg.turtles = turtles_;
        turtle_array_publisher_->publish(turtle_array_msg);
    }

    /**
     * A callback function that is responsible for handling the catch turtle service requests
     * by deleting the requested turtle from the list if found.
     * 
     * param request The request object that contains the name of the turtle to be caught, 
     *               which is used to find and remove the turtle from the list of alive turtles if it exists.
     * 
     * param response The response object that is used to indicate whether the turtle was successfully caught or not, 
     *                and to provide a message about the result of the catch attempt.
     */
    void catch_turtle_server_callback(const std::shared_ptr<my_robot_interfaces::srv::CatchTurtle::Request> request, 
                            const std::shared_ptr<my_robot_interfaces::srv::CatchTurtle::Response> response)
    {
        auto it = std::find_if(turtles_.begin(), turtles_.end(), [&request](const my_robot_interfaces::msg::Turtle& turtle) {
            return turtle.name == request->name;
        });

        if (it == turtles_.end()) {
            response->success = false;
            // response->message = "Turtle not found: " + request->name;
            return;
        }
        
        response->success = true;
        response->message = "Caught turtle: " + request->name;

        // Call the kill service to remove the turtle from the turtlesim game
        call_kill_service(request->name);
        turtles_.erase(it);
        RCLCPP_INFO(this->get_logger(), "Killed turtle: %s", it->name);
    }

    /**
     * A function that is responsible for calling the kill service to remove a turtle from the turtlesim game when it is caught.
     * 
     * param turtle_name The name of the turtle to be killed, 
     *                   which is passed to the kill service request 
     *                   to identify which turtle to remove from the game.
     */
    void call_kill_service(const std::string& turtle_name)
    {
        auto kill_client = this->create_client<turtlesim::srv::Kill>("/kill");
        auto kill_request = std::make_shared<turtlesim::srv::Kill::Request>();
        kill_request->name = turtle_name;

        while (!kill_client->wait_for_service(std::chrono::seconds(1))) 
        {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
                return;
            }
            RCLCPP_INFO(this->get_logger(), "Kill service not available, waiting again...");
        }

        auto result_future = kill_client->async_send_request(kill_request);
        // RCLCPP_INFO(this->get_logger(), "Called kill service for turtle: %s", turtle_name.c_str());
    }

    rclcpp::Publisher<my_robot_interfaces::msg::TurtleArray>::SharedPtr turtle_array_publisher_;

    rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawn_client_;
    rclcpp::Client<turtlesim::srv::Kill>::SharedFuture client_for_kill_service;

    rclcpp::Service<my_robot_interfaces::srv::CatchTurtle>::SharedPtr catch_turtle_server_;

    my_robot_interfaces::msg::Turtle turtle;
    std::vector<my_robot_interfaces::msg::Turtle> turtles_;

    rclcpp::TimerBase::SharedPtr spawn_timer_;
    rclcpp::TimerBase::SharedPtr turtle_array_timer_;


    float new_turtle_x, new_turtle_y;
    int spawn_frequency_;
    std::string turtle_name_prefix_;
    std::string new_turtle_name;
    int turtle_counter_;


};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TurtleSpawnerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
