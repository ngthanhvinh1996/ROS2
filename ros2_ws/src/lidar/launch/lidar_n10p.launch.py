import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def launch_setup(context, *args, **kwargs):
    config = os.path.join(
        get_package_share_directory('lidar'),
        'params',
        'lidar_n10p.yaml'
    )

    debug_mode = LaunchConfiguration('debug').perform(context)

    debug_prefix = 'gnome-terminal --geometry=120x40 -- gdb -ex "layout src" -ex run --args'

    node_prefix = debug_prefix if debug_mode.lower() == 'true' else None

    valgrind_prefix = 'valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes'

    return [
        Node(
            package='lidar',
            executable='lidar_node',
            name='lidar_node',
            output='screen',
            prefix=node_prefix,
            parameters=[config],
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