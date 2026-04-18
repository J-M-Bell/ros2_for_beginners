#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from example_interfaces.msg import Int64
from my_robot_interfaces.msg import LedPanelState
from my_robot_interfaces.srv import SetLed

"""
A node that publishes the led panel state to the 
'led_panel_state' topic.
"""
class LedPanelNode(Node):
    """
    A function that initializes the LedPanelNode class to create the
    led panel int array and publish its state to the 'led_panel_state' topic.
    """
    def __init__(self):
        super().__init__("led_panel")
        self.declare_parameter("led_states", [0, 0, 0])
        self.led_panel_ =  self.get_parameter("led_states").value
        self.led_panel_publisher_ = self.create_publisher(LedPanelState, "led_panel_state", 10)
        self.led_panel_server = self.create_service(SetLed, "set_led", self.led_panel_server_callback)
        self.timer_ = self.create_timer(1.0, self.publish_led_panel_states)

    """
    A publisher function that publishes the 'led_panel_' variable to
    the 'led_panel_state' topic.
    """
    def publish_led_panel_states(self):
        states = LedPanelState()
        states.led_panel_states = self.led_panel_
        self.led_panel_publisher_.publish(states)

    """
    A service callback function that updates the 'led_panel_' variable based on the
    request data received from the 'led_panel_server' service.

    param request: The request data containing the led number and led state to update.
    param response: The response data indicating the success of the update operation.
    
    return response: The response data indicating the success of the update operation.
    """
    def led_panel_server_callback(self, request, response):
        led_number = request.led_number
        led_state = request.led_state

        if (led_number <= 0 or led_number > len(self.led_panel_) or led_state < 0 or led_state > 1):
            response.result = False
            self.get_logger().info("Incorrect input provided...")
            return response
        
        self.led_panel_[led_number-1] = led_state
        response.result = True
        return response

def main(args=None):
    rclpy.init(args=args)
    node = LedPanelNode()
    rclpy.spin(node)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
