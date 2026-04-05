from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def launch_setup(context, *args, **kwargs):
    debug_mode = LaunchConfiguration('debug').perform(context)

    debug_prefix = 'gnome-terminal --geometry=120x40 -- gdb -ex "layout src" -ex run --args'

    node_prefix = debug_prefix if debug_mode.lower() == 'true' else None

    return [
        Node(
            package='lidar',
            executable='lidar_node',
            name='lidar_node',
            output='screen',
            prefix=node_prefix,
            parameters=[
                {'port': '/dev/ttyUSB0'}, 
                {'baud_rate': 460800},
            ],
        )
    ]

def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'debug',
            default_value='false',
            description='Run Debug Mode'
        ),
        OpaqueFunction(function=launch_setup)
    ])