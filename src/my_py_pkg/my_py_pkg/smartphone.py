#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from example_interfaces.msg import String

"""
A node that prints a string to the 'robot_news' topic.
"""
class SmartphoneNode(Node):
    """
    A function that creates the subscriber to the 'robot_news' topic.
    """
    def __init__(self):
        super().__init__("smartphone")
        self.subscriber_ = self.create_subscription(String, "robot_news", self.callback_robot_news, 10)
        self.get_logger().info("Smartphone has been started.")
    
    """
    A callback function that prints the message from the 'robot_news' topic
    to the user.
    """
    def callback_robot_news(self, msg: String):
        self.get_logger().info(msg.data)


def main(args=None):
    rclpy.init(args=args)
    node = SmartphoneNode()
    rclpy.spin(node)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
