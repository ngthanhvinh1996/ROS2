from launch import LaunchDescription
from launch_ros.actions import Node
import os

def generate_launch_description():
    dnn_model_path = "/opt/hobot/model/x5/basic/yolov5x_672x672_nv12.bin"
    dnn_config_path = "/home/sunrise/workdir/ROS2/ros2_ws/src/robot_camera/models/yolov5workconfig.json"

    return LaunchDescription([
        # ---------------------------------------------------------
        # NODE 1: CAMERA (Nguồn ảnh gốc)
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
                {"image_width": 1920},
                {"image_height": 1080},
                {"framerate": 30.0},
                {"rotation": 180.0},
                {"gdc_enable": False},
                {"cal_rotation": 90.0},
                {"lpwm_enable": False},
                {"frame_ts_type": "realtime"},
                {"out_format": "nv12"},
                {"channel": 2},
                {"channel2": 0},
            ]
        ),

        # ---------------------------------------------------------
        # NODE 2: AI INFERENCE (YOLOv5)
        # ---------------------------------------------------------
        Node(
            package='dnn_node_example',
            executable='example',
            output='screen',
            parameters=[
                {"config_file": dnn_config_path},
                {"model_file": dnn_model_path},
                {"dnn_parser": "yolov5"},
                {"msg_pub_topic_name": "ai_msg_mono2d"},
                {"feed_type": 1},
                {"is_shared_mem_sub": 0},
                {"ros_img_topic_name": "/image_raw"}
            ],
        ),

        # ---------------------------------------------------------
        # NODE 3: VISUALIZATION (Vẽ hộp lên ảnh)
        # ---------------------------------------------------------
        # Node(
        #     package='hobot_visualization',
        #     executable='hobot_visualization',
        #     output='screen',
        #     parameters=[
        #         {"msg_pub_topic_name": "ai_image_processed"} 
        #     ],
        #     remappings=[
        #         ('/dnn_data', '/ai_msg_mono2d'),
        #         ('/image_raw', '/image_raw')
        #     ]
        # )
    ])