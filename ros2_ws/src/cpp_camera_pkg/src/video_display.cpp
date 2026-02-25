#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.h"
#include "opencv2/opencv.hpp"
#include "image_transport/image_transport.hpp"

class VideoDisplay : public rclcpp::Node
{
public:
  VideoDisplay() : Node("video_display_node")
  {
    cv::namedWindow("ROS 2 Camera Stream", cv::WINDOW_AUTOSIZE);

    // subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
    //   "camera/image_raw", 
    //   // "/image_left_raw",
    //   10, 
    //   std::bind(&VideoDisplay::topic_callback, this, std::placeholders::_1));
    
    image_transport::ImageTransport it(Node);

    sub_ = image_transport::create_subscription(
      this, 
      "/camera/image_raw", 
      std::bind(&VideoDisplay::const_topic_callback, this, std::placeholders::_1), 
      "compressed", // Gợi ý dùng nén (nếu muốn)
      rmw_qos_profile_sensor_data // QoS Best Effort
    );

    RCLCPP_INFO(this->get_logger(), "Dang cho du lieu video...");
  }

  ~VideoDisplay()
  {
    cv::destroyAllWindows();
  }

private:
  void topic_callback(const sensor_msgs::msg::Image::SharedPtr msg)
  {
    try {
      cv::Mat frame = cv_bridge::toCvCopy(msg, "bgr8")->image;

      cv::imshow("ROS 2 Camera Stream", frame);
      
      cv::waitKey(1); 
    } 
    catch (cv_bridge::Exception& e) {
      RCLCPP_ERROR(this->get_logger(), "Loi cv_bridge: %s", e.what());
    }
  }

  void const_topic_callback(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
  {
    try {
      // Chuyển đổi ROS Image -> OpenCV Mat
      // Dùng bgr8 để hiển thị đúng màu
      cv::Mat frame = cv_bridge::toCvShare(msg, "bgr8")->image;

      if (frame.empty()) {
        RCLCPP_WARN(this->get_logger(), "Received empty image");
        return;
      }
      // cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
      // Hiển thị ảnh
      cv::imshow("Camera View", frame);
      cv::waitKey(1); // Bắt buộc phải có để OpenCV vẽ hình
    } 
    catch (cv_bridge::Exception& e) {
      RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
  image_transport::Subscriber sub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<VideoDisplay>());
  rclcpp::shutdown();
  return 0;
}
