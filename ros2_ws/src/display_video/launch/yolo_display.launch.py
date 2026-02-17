from launch import LaunchDescription
from launch_ros.actions import Node
import os

def generate_launch_description():
    return LaunchDescription([
        # ---------------------------------------------------------
        # NODE 1: AI Inference (Chạy YOLO trên BPU)
        # ---------------------------------------------------------
        Node(
            package='dnn_node_example',
            executable='example',
            output='screen',
            parameters=[
                {"config_file": "/home/sunrise/workdir/models/yolov5workconfig.json"},
                {"model_file": "/home/sunrise/workdir/models/yolov5s_672x672_nv12.bin"},
                {"dnn_parser": "yolov5"},
                {"msg_pub_topic_name": "ai_msg_mono2d"} # Topic chứa kết quả (box, class)
            ],
            remappings=[
                # DNN node cần topic tên "image_raw", nếu camera bạn pub tên khác thì sửa bên phải
                ('/image_raw', '/image_raw') 
            ]
        ),

        # ---------------------------------------------------------
        # NODE 2: Visualization (Vẽ bounding box lên ảnh)
        # ---------------------------------------------------------
        Node(
            package='hobot_visualization',
            executable='hobot_visualization',
            output='screen',
            parameters=[
                {"msg_pub_topic_name": "hobot_visualization_img"} # Topic ảnh đầu ra đã vẽ hình
            ],
            remappings=[
                ('/dnn_data', '/ai_msg_mono2d'), # Lấy dữ liệu box từ Node 1
                ('/image_raw', '/image_raw')     # Lấy ảnh gốc từ Camera
            ]
        ),

        # ---------------------------------------------------------
        # NODE 3: Display (Node của bạn - Hiển thị lên màn hình)
        # ---------------------------------------------------------
        Node(
            package='display_video',
            executable='display_video_node',
            output='screen',
            remappings=[
                # QUAN TRỌNG: Ép node của bạn lắng nghe ảnh ĐÃ VẼ HÌNH
                # thay vì nghe ảnh gốc.
                ('/image_raw', '/hobot_visualization_img')
            ]
        )
    ])