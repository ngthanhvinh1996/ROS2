#include <cstdio>
#include <chrono>
#include <memory>
#include <string>

// ROS 2 Headers
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/header.hpp"
#include "cv_bridge/cv_bridge.h" // Cầu nối quan trọng giữa OpenCV và ROS
#include "image_transport/image_transport.hpp" // Để gửi ảnh hiệu quả hơn

// OpenCV Headers
#include <opencv2/opencv.hpp>

using namespace std::chrono_literals;

class OpiCameraNode : public rclcpp::Node {
public:
    OpiCameraNode() : Node("opi_camera_node") {
        // 1. Khai báo Publisher
        // Dùng image_transport để sau này có thể nén ảnh (compressed) nếu muốn
        publisher_ = image_transport::create_publisher(this, "camera/image_raw");

        // 2. Cấu hình GStreamer Pipeline (Hardware ISP)
        // Đây là chuỗi pipeline bạn đã test thành công ở các bước trước
        // Lưu ý: format=NV12 (từ ISP) -> videoconvert -> BGR (cho OpenCV)
        std::string pipeline = "v4l2src device=/dev/video11 ! "
                               "video/x-raw, format=NV12, width=800, height=600 ! "
                               "videoconvert ! "
                               "video/x-raw, format=BGR ! "
                               "appsink drop=1";

        RCLCPP_INFO(this->get_logger(), "Dang mo Camera qua GStreamer...");
        cap_.open(pipeline, cv::CAP_GSTREAMER);

        if (!cap_.isOpened()) {
            RCLCPP_ERROR(this->get_logger(), "KHONG THE MO CAMERA! Hay kiem tra /dev/video11");
            return;
        }

        RCLCPP_INFO(this->get_logger(), "Camera da san sang! Dang publish tai topic /camera/image_raw");

        // 3. Tạo Timer để đọc ảnh liên tục (30 FPS -> 33ms)
        timer_ = this->create_wall_timer(
            33ms, std::bind(&OpiCameraNode::timer_callback, this));
    }

private:
    void timer_callback() {
        cv::Mat frame;
        // Đọc từ Hardware ISP
        cap_ >> frame;

        if (frame.empty()) {
            RCLCPP_WARN(this->get_logger(), "Frame bi rong (Dropped frame)");
            return;
        }

        // Chuyển đổi từ OpenCV Mat -> ROS Image Message
        std_msgs::msg::Header header;
        header.stamp = this->get_clock()->now();
        header.frame_id = "camera_link"; // Tên frame để hiển thị trong TF

        // Dùng cv_bridge để đóng gói
        sensor_msgs::msg::Image::SharedPtr msg = cv_bridge::CvImage(
            header, 
            "bgr8", // Định dạng màu của OpenCV hiện tại
            frame
        ).toImageMsg();

        // Gửi đi
        publisher_.publish(*msg);
    }

    image_transport::Publisher publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    cv::VideoCapture cap_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<OpiCameraNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}