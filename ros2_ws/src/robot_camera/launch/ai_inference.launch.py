from launch import LaunchDescription
from launch_ros.actions import Node
import os

def generate_launch_description():
    dnn_model_path = "/opt/hobot/model/x5/basic/yolov8_640x640_nv12.bin"
    dnn_config_path = "/home/sunrise/workdir/ROS2/ros2_ws/src/robot_camera/models/yolov8workconfig.json"

    return LaunchDescription([
        # ---------------------------------------------------------
        # NODE 1: CAMERA
        # ---------------------------------------------------------
        Node(
            package='robot_camera',
            executable='robot_camera',
            name='robot_camera_node',
            output='screen',
            parameters=[
                {"video_device": "SC230AI"},
                {"device_mode": "single"},
                {"dual_combine": 0},
                {"image_width": 960},
                {"image_height": 544},
                {"framerate": 30.0},
                {"rotation": 180.0},
                {"gdc_enable": False},
                {"cal_rotation": 90.0},
                {"lpwm_enable": False},
                {"frame_ts_type": "realtime"},
                {"out_format": "nv12"},
                {"channel": 2},
                {"channel2": 0},
                {"io_method": "shared_mem"},
            ]
        ),

        # ---------------------------------------------------------
        # NODE 2: AI INFERENCE (YOLOv8)
        # ---------------------------------------------------------
        Node(
            package='dnn_node_example',
            executable='example',
            output='screen',
            parameters=[
                {"config_file": dnn_config_path},
                {"model_file": dnn_model_path},
                {"dnn_Parser": "yolov8"},
                {"msg_pub_topic_name": "hobot_dnn_detection"},
                {"feed_type": 1},
                {"is_shared_mem_sub": 1},
                {"ros_img_topic_name": "/hbmem_img"}
            ],
        ),

        # ---------------------------------------------------------
        # NODE 3: CODEC
        # ---------------------------------------------------------
        Node(
            package='hobot_codec',
            executable='hobot_codec_republish',
            output='screen',
            name='hobot_codec_node',
            parameters=[
                {"codec_in_mode": "shared_mem"},
                {"codec_out_mode": "ros"},
                {"codec_sub_topic": "/hbmem_img"},
                {"codec_pub_topic": "/image"}
            ],
        )
    ])