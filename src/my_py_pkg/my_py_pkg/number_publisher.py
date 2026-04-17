#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from example_interfaces.msg import Int64
from example_interfaces.srv import SetBool

"""
A ROS2 Node that publishes a number to a topic every 1 second.
"""
class NumberPublisherNode(Node):
    """
    A function that initializes the number publisher and timer for publishing a number
    to the 'number' topic
    """
    def __init__(self):
        super().__init__("number_publisher")
        self.publisher_ = self.create_publisher(Int64, "number", 10)
        self.timer_ = self.create_timer(1, self.publish_number)
        self.get_logger().info("Number publisher has been started.")
    
    """
    A function that publishes an Int64 number to the 'number' topic
    """
    def publish_number(self):
        msg = Int64()
        msg.data = 2
        self.publisher_.publish(msg)


"""
The main function responsible for continuously spinning the NumberPublisherNode.
"""
def main(args=None):
    rclpy.init(args=args)
    node = NumberPublisherNode()
    rclpy.spin(node)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
