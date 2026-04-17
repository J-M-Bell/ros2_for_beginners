#!/usr/bin/env python3
import rclpy
from rclpy.node import Node

"""
A node that prints a string to every 1 second.
"""
class MyNode(Node):

    """
    A function that creates a timer to print a string and to keep a
    counter to display to the user.
    """
    def __init__(self):
        super().__init__("py_test")
        self.counter_ = 0
        self.get_logger().info("Hello world")
        self.create_timer(1.0, self.timer_callback)

    """
    A callback function that displays a string that contains a counter.
    """
    def timer_callback(self):
        self.get_logger().info("Hello aaaaa " + str(self.counter_))
        self.counter_ += 1


def main(args=None):
    rclpy.init(args=args)
    node = MyNode()
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == "__main__":
    main()