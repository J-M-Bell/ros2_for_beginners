#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from example_interfaces.msg import String

"""
A node that simulates a robot news station by publishing news messages to a topic.
The node publishes a message every 0.5 seconds, 
which includes the robot's name and a greeting.
"""
class RobotNewsStationNode(Node):
    """
    A function that initializes the robot news station,
    sets the robot's name, creates a publisher for the 'robot_news' topic,
    and sets a timer to publish news messages at regular intervals.
    """
    def __init__(self):
        super().__init__("robot_news_station")
        self.declare_parameter("robot_name", "C3PO")
        self.robot_name_ = self.get_parameter("robot_name").value
        self.publisher_ = self.create_publisher(String, "robot_news", 10)
        self.timer_ = self.create_timer(0.5, self.publish_news)
        self.get_logger().info("Robot News Station has been started.")

    """
    A function that creates a String message containing the robot's name and a greeting,
    and publishes it to the 'robot_news' topic.
    """
    def publish_news(self):
        msg = String()
        msg.data = "Hello, this is " + self.robot_name_ + " from the robot news station."
        self.publisher_.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    node = RobotNewsStationNode() 
    rclpy.spin(node)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
