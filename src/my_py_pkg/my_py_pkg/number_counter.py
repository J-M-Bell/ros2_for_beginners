#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from example_interfaces.msg import Int64
from example_interfaces.srv import SetBool

"""
A ROS2 Node that subscribes to the 'number' topic and 
counts the number of messages received and publishes the count.
"""
class NumberCounterNode(Node):
    
    """
    A function that initializes the number counter, 
    subscription to the 'number' topic,
    and publisher for the 'number_count' topic.
    """
    def __init__(self):
        super().__init__("number_counter")
        self.subscription_ = self.create_subscription(Int64, "number", self.number_callback, 10)
        self.publisher_ = self.create_publisher(Int64, "number_count", 10)
        self.server_ = self.create_service(SetBool, "reset_counter", self.reset_counter_callback)
        self.count_ = 0
        self.get_logger().info("Number counter has been started.")
    
    """
    A callback function that increments the count each time 
    a message is received on the 'number' topic
    and logs the current count.

    param msg: Int64 - The message received from the 'number' topic.
    """
    def number_callback(self, msg):
        message = msg.data
        self.count_ += 1
        msg.data = msg.data + (self.count_ * 2)
        self.publisher_.publish(msg)

    """
    A callback function that resets the counter when 
    a request is received on the 'reset_counter' service.
    """
    def reset_counter_callback(self, request, response):
        if request.data:
            self.count_ = 0
            response.success = True
            response.message = "Counter reset successfully."
        else:
            response.success = False
            response.message = "Counter reset failed. Request data must be True."
        return response
    
def main(args=None):
    rclpy.init(args=args)
    node = NumberCounterNode()
    rclpy.spin(node)
    rclpy.shutdown()