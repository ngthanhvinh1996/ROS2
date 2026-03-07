from launch import LaunchDescription
from launch_ros.actions import Node
import os

def generate_launch_description():
    return LaunchDescription([
        # ---------------------------------------------------------
        # NODE 1: DISPLAY
        # ---------------------------------------------------------
        Node(
            package='display_video',
            executable='display_video',
            name='display_video_node',
            output='screen',
            parameters=[
                {"image_out_type": "jpeg"},
                {"image_sub_topic": "/image"},
                {"ai_sub_topic": "hobot_dnn_detection"}
            ]
        )
    ])