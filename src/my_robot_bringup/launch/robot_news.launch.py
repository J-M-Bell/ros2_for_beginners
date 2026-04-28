from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    ld = LaunchDescription()

    # Define the robot news station nodes with different robot names
    robot_news_station_R2D2 = Node(
        package="my_py_pkg",
        executable="robot_news_station",
        name="robot_news_station_r2d2",
        parameters=[{"robot_name": "R2D2"}]
    )

    robot_news_station_C3PO = Node(
        package="my_py_pkg",
        executable="robot_news_station",
        name="robot_news_station_c3po",
        parameters=[{"robot_name": "C3PO"}]
    )

    robot_news_station_Sunny = Node(
        package="my_py_pkg",
        executable="robot_news_station",
        name="robot_news_station_sunny",
        parameters=[{"robot_name": "Sunny"}]
    )

    robot_news_station_WallE = Node(
        package="my_py_pkg",
        executable="robot_news_station",
        name="robot_news_station_walle",
        parameters=[{"robot_name": "WallE"}]
    )

    robot_news_station_Abraham = Node(
        package="my_py_pkg",
        executable="robot_news_station",
        name="robot_news_station_abraham",
        parameters=[{"robot_name": "Abraham"}]
    )

    # Define the smartphone node
    smartphone = Node(
        package="my_py_pkg",
        executable="smartphone",
        name="smartphone"
    )
    
    # Add all nodes to the launch description
    ld.add_action(robot_news_station_R2D2)
    ld.add_action(robot_news_station_C3PO)
    ld.add_action(robot_news_station_Sunny)
    ld.add_action(robot_news_station_WallE)
    ld.add_action(robot_news_station_Abraham)
    ld.add_action(smartphone)

    return ld