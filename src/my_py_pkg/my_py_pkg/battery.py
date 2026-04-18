#!/usr/bin/env python3
from functools import partial

import rclpy
from rclpy.node import Node
from my_robot_interfaces.srv import SetLed

"""
A node that acts as a battery to power on an LED depending on
if the battery is fully powered.
"""
class BatteryNode(Node):
    def __init__(self):
        super().__init__("battery")
        self.battery_level_empty = False
        self.led_panel_client = self.create_client(SetLed, "set_led")
        self.timer_ = self.create_timer(0.1, self.battery_timer_callback)
        seconds, nanoseconds = self.get_clock().now().seconds_nanoseconds()
        self.last_time_battery_changed = seconds + nanoseconds / 1000000000.0

    """
    A timer callback that simulates the battery level 
    by using timing methods to considered the battery empty and turn an LED on
    when the timer reaches 4 seconds and the battery full and LED off
    when the timer reaches 6 seconds.
    """
    def battery_timer_callback(self):
        seconds, nanoseconds = self.get_clock().now().seconds_nanoseconds()
        self.time_now = seconds + nanoseconds / 1000000000.0
        if ((self.time_now - self.last_time_battery_changed > 4.0) and not self.battery_level_empty):
            self.battery_level_empty = True
            self.get_logger().info("Battery level is empty")
            self.call_set_led(1, True)
            self.last_time_battery_changed = self.time_now
        elif ((self.time_now - self.last_time_battery_changed > 6.0) and self.battery_level_empty):
            self.battery_level_empty = False
            self.get_logger().info("Battery level is full")
            self.call_set_led(1, False)
            self.last_time_battery_changed = self.time_now


    
    """
    A service client callback that calls the SetLed service to turn on or off the LED depending on the battery level.
    """
    def call_set_led(self, led_number, led_state):
        while not self.led_panel_client.wait_for_service(1.0):
            self.get_logger().warn("Waiting for SetLed server...")

        request = SetLed.Request()
        request.led_number = led_number
        request.led_state = led_state

        self.led_panel_client.call_async(request)


def main(args=None):
    rclpy.init(args=args)
    node = BatteryNode()
    rclpy.spin(node)
    rclpy.shutdown()


if __name__ == "__main__":
    main()
