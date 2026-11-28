import launch
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    container = ComposableNodeContainer(
        name='blob_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            ComposableNode(
                package='lifecycle_composition',
                plugin='lifecycle_composition::BlobPublisher',
                name='blob_pub',
                parameters=[{'publish_hz': 100.0, 'payload_size': 65536}],
                extra_arguments=[{'use_intra_process_comms': True}],
            ),
            ComposableNode(
                package='lifecycle_composition',
                plugin='lifecycle_composition::BlobSubscriber',
                name='blob_sub',
                parameters=[{'log_every': 200}],
                extra_arguments=[{'use_intra_process_comms': True}],
            ),
        ],
        output='screen',
    )
    return launch.LaunchDescription([container])