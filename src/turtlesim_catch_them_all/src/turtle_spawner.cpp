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
        spawn_frequency_ = this->declare_parameter("spawn_frequency", 2);
        turtle_name_prefix_ = this->declare_parameter("turtle_name_prefix", "");

        turtle_counter_ = 2;

        // Create a client for the spawn service
        spawn_client_ = this->create_client<turtlesim::srv::Spawn>("/spawn");
        spawn_timer_ = this->create_timer(std::chrono::duration<double>(1.0 / spawn_frequency_), std::bind(&TurtleSpawnerNode::callSpawnService, this));


        // Create a publisher for the array of alive turtles
        turtle_array_publisher_ = this->create_publisher<my_robot_interfaces::msg::TurtleArray>("/alive_turtles", 10);
        turtle_array_timer_ = this->create_wall_timer(std::chrono::seconds(1), std::bind(&TurtleSpawnerNode::publishTurtleArray, this));

        // Create a service to catch turtles
        catch_turtle_server_ = this->create_service<my_robot_interfaces::srv::CatchTurtle>(
            "catch_turtle",
            std::bind(&TurtleSpawnerNode::catchTurtleCallback, this, _1, _2)
        );

    }


private:
    /**
     * A timer callback that is responsible for calling the spawn service to spawn a new turtle at a random location every 5 seconds.
     */
    void callSpawnService()
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
        request->name = turtle_name_prefix_ + "turtle" + std::to_string(turtle_counter_);
        turtle_counter_ += 1;

        // Store the new turtle's position to be added to array
        new_turtle_x = request->x;
        new_turtle_y = request->y;
        

        // Call the service to spawn the turtle
        auto result_future = spawn_client_->async_send_request(request, std::bind(&TurtleSpawnerNode::callbackSpawnTurtles, this, _1));
        // RCLCPP_INFO(this->get_logger(), "Spawned turtle at (%.2f, %.2f)", request->x, request->y);
    }

    /**
     * A callback function that is called when the spawn service returns a response.
     */
    void callbackSpawnTurtles(rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future)
    {
        auto response = future.get();
        // RCLCPP_INFO(this->get_logger(), "Spawned turtle %s", response->name.c_str());
        
        // Add the new turtle to the list of alive turtles
        new_turtle_name = response->name;
        my_robot_interfaces::msg::Turtle new_turtle;
        new_turtle.name = response->name;
        new_turtle.x = new_turtle_x;
        new_turtle.y = new_turtle_y;
        turtles_.push_back(new_turtle);
    }

    /**
     * A timer callback that is responsible for publishing the array of alive turtles every second.
     */
    void publishTurtleArray()
    {
        auto turtle_array_msg = my_robot_interfaces::msg::TurtleArray();
        turtle_array_msg.turtles = turtles_;
        turtle_array_publisher_->publish(turtle_array_msg);
    }

    /**
     * A callback function that is responsible for handling the catch turtle service requests
     * by deleting the requested turtle from the list if found.
     */
    void catchTurtleCallback(const std::shared_ptr<my_robot_interfaces::srv::CatchTurtle::Request> request, 
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
        callKillService(request->name);
        turtles_.erase(it);
    }

    void callKillService(const std::string& turtle_name)
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

    
    rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawn_client_;
    rclcpp::Client<turtlesim::srv::Kill>::SharedFuture client_for_kill_service;
    rclcpp::Publisher<my_robot_interfaces::msg::TurtleArray>::SharedPtr turtle_array_publisher_;
    rclcpp::Service<my_robot_interfaces::srv::CatchTurtle>::SharedPtr catch_turtle_server_;
    my_robot_interfaces::msg::Turtle turtle;
    float new_turtle_x, new_turtle_y;
    std::string new_turtle_name;
    std::vector<my_robot_interfaces::msg::Turtle> turtles_;
    rclcpp::TimerBase::SharedPtr spawn_timer_;
    rclcpp::TimerBase::SharedPtr turtle_array_timer_;
    int spawn_frequency_;
    std::string turtle_name_prefix_;
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
