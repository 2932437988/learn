import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('configurable_node'),
        'config',
        'params.yaml',
    )

    robot_name_arg = DeclareLaunchArgument(
        'robot_name',
        default_value='from_launch',
        description='Name of the robot',
    )

    return LaunchDescription([
        robot_name_arg,
        Node(
            package='configurable_node',
            executable='configurable_node',
            name='configurable_node',
            parameters=[
                config,
                {'robot_name': LaunchConfiguration('robot_name')},
            ],
            output='screen',
        ),
        Node(
            package='robot_status_pubsub',
            executable='status_publisher',
            name='status_publisher',
            parameters=[{'robot_name': LaunchConfiguration('robot_name')}],
            output='screen',
        ),
    ])