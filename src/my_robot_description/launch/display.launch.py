from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import Command
import os
from ament_index_python.packages import get_package_share_path

def generate_launch_description():
    # Get the path to the robot description package
    pkg_share = get_package_share_path('my_robot_description')

    # Define the path to the URDF file
    urdf_file = os.path.join(pkg_share, 'urdf', 'my_robot.urdf.xacro')

    #Set up the robot description parameter by processing the xacro file
    robot_description = ParameterValue(Command(['xacro ', urdf_file]), value_type=str)

    # Start the robot_state_publisher node to publish the robot's state to TF
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': robot_description
        }]
    )

    # Run the joint_state_publisher_gui to allow interactive control of the robot's joints
    joint_state_publisher_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
    )

    # Create a Node action to launch RViz2 with the robot description
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        arguments=['-d', os.path.join(pkg_share, 'rviz', 'my_robot_config.rviz')],
    )


    return LaunchDescription([
        robot_state_publisher_node,
        joint_state_publisher_gui_node,
        rviz_node
    ])