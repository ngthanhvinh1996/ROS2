// #include "rclcpp/rclcpp.hpp"
// #include "sensor_msgs/msg/image.hpp"
// #include "cv_bridge/cv_bridge.h"
// #include "opencv2/opencv.hpp"

// class DisplayNode : public rclcpp::Node
// {
// public:
//   DisplayNode() : Node("display_video_node")
//   {
//     rclcpp::QoS qos_profile(10);
//     qos_profile.reliability(rclcpp::ReliabilityPolicy::BestEffort);

//     device_mode_ = "single";
//     dual_combine_ = 0;

//     this->declare_parameter<std::string>("device_mode", device_mode_);
//     this->declare_parameter<int>("dual_combine", dual_combine_);

//     this->get_parameter<std::string>("device_mode", device_mode_);
//     this->get_parameter<int>("dual_combine", dual_combine_);

//     if("single" == device_mode_)
//     {
//       subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
//       "ai_image_processed",
//       qos_profile,
//       std::bind(&DisplayNode::topic_callback, this, std::placeholders::_1));

//       RCLCPP_INFO(this->get_logger(), "Display Node single has been started. Waiting for images...");
    
//       cv::namedWindow("Robot Camera Stream", cv::WINDOW_AUTOSIZE);
//     }
//     else
//     {
//       if(0 == dual_combine_)
//       {
//         dual_left_subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
//         "image_left_raw",
//         qos_profile,
//         std::bind(&DisplayNode::dual_left_topic_callback, this, std::placeholders::_1));

//         cv::namedWindow("Robot Camera Stream Left", cv::WINDOW_NORMAL);

//         dual_right_subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
//         "image_right_raw",
//         qos_profile,
//         std::bind(&DisplayNode::dual_right_topic_callback, this, std::placeholders::_1));

//         RCLCPP_INFO(this->get_logger(), "Display Node dual_combine_ 0 has been started. Waiting for images...");

//         cv::namedWindow("Robot Camera Stream Right", cv::WINDOW_NORMAL);
//       }
//       else
//       {
//         RCLCPP_INFO(this->get_logger(), "Don't support dual mode:%d", dual_combine_);
//         exit(0);
//       }
//     }    
//   }

// private:
//   void topic_callback(const sensor_msgs::msg::Image::SharedPtr msg)
//   {
//     cv::Mat display_frame;

//     try {
//       if (msg->encoding == "nv12") {
//         cv::Mat nv12_mat(msg->height * 3 / 2, msg->width, CV_8UC1, const_cast<uint8_t*>(msg->data.data()));

//         cv::cvtColor(nv12_mat, display_frame, cv::COLOR_YUV2BGR_NV12);
//       } 
//       else {
//         display_frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
//       }

//       if (display_frame.empty()) {
//         RCLCPP_WARN(this->get_logger(), "Frame is empty!");
//         return;
//       }

//       cv::imshow("Robot Camera Stream", display_frame);
//       cv::waitKey(1);

//       static int count = 0;
//       if (++count % 60 == 0) {
//         RCLCPP_INFO(this->get_logger(), "Displaying: %dx%d (%s)", 
//                     msg->width, msg->height, msg->encoding.c_str());
//       }

//     } catch (cv::Exception& e) {
//       RCLCPP_ERROR(this->get_logger(), "OpenCV Error: %s", e.what());
//     } catch (cv_bridge::Exception& e) {
//       RCLCPP_ERROR(this->get_logger(), "cv_bridge Error: %s", e.what());
//     }
//   }

//   void dual_left_topic_callback(const sensor_msgs::msg::Image::SharedPtr msg)
//   {
//     cv::Mat display_frame;

//     try {
//       if (msg->encoding == "nv12") {
//         cv::Mat nv12_mat(msg->height * 3 / 2, msg->width, CV_8UC1, const_cast<uint8_t*>(msg->data.data()));

//         cv::cvtColor(nv12_mat, display_frame, cv::COLOR_YUV2BGR_NV12);
//       } 
//       else {
//         display_frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
//       }

//       if (display_frame.empty()) {
//         RCLCPP_WARN(this->get_logger(), "Frame is empty!");
//         return;
//       }

//       cv::imshow("Robot Camera Stream Left", display_frame);
//       cv::waitKey(1);

//       static int count = 0;
//       if (++count % 60 == 0) {
//         RCLCPP_INFO(this->get_logger(), "Displaying: %dx%d (%s)", 
//                     msg->width, msg->height, msg->encoding.c_str());
//       }

//     } catch (cv::Exception& e) {
//       RCLCPP_ERROR(this->get_logger(), "OpenCV Error: %s", e.what());
//     } catch (cv_bridge::Exception& e) {
//       RCLCPP_ERROR(this->get_logger(), "cv_bridge Error: %s", e.what());
//     }
//   }

//   void dual_right_topic_callback(const sensor_msgs::msg::Image::SharedPtr msg)
//   {
//     cv::Mat display_frame;

//     try {
//       if (msg->encoding == "nv12") {
//         cv::Mat nv12_mat(msg->height * 3 / 2, msg->width, CV_8UC1, const_cast<uint8_t*>(msg->data.data()));

//         cv::cvtColor(nv12_mat, display_frame, cv::COLOR_YUV2BGR_NV12);
//       } 
//       else {
//         display_frame = cv_bridge::toCvCopy(msg, "bgr8")->image;
//       }

//       if (display_frame.empty()) {
//         RCLCPP_WARN(this->get_logger(), "Frame is empty!");
//         return;
//       }

//       cv::imshow("Robot Camera Stream Right", display_frame);
//       cv::waitKey(1);

//       static int count = 0;
//       if (++count % 60 == 0) {
//         RCLCPP_INFO(this->get_logger(), "Displaying: %dx%d (%s)", 
//                     msg->width, msg->height, msg->encoding.c_str());
//       }

//     } catch (cv::Exception& e) {
//       RCLCPP_ERROR(this->get_logger(), "OpenCV Error: %s", e.what());
//     } catch (cv_bridge::Exception& e) {
//       RCLCPP_ERROR(this->get_logger(), "cv_bridge Error: %s", e.what());
//     }
//   }

//   rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
//   rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr dual_left_subscription_;
//   rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr dual_right_subscription_;
//   std::string device_mode_;
//   int dual_combine_;
// };

// int main(int argc, char * argv[])
// {
//   rclcpp::init(argc, argv);
//   rclcpp::spin(std::make_shared<DisplayNode>());
//   rclcpp::shutdown();
//   return 0;
// }

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <ai_msgs/msg/perception_targets.hpp> 

// Thư viện đồng bộ hóa thời gian
#include <message_filters/subscriber.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <message_filters/synchronizer.h>

// Thư viện chuyển đổi ROS Image <-> OpenCV Mat
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

using std::placeholders::_1;
using std::placeholders::_2;

class AiDisplayNode : public rclcpp::Node {
public:
    AiDisplayNode() : Node("ai_display_custom_node") {
        // QoS profile
        rmw_qos_profile_t qos_profile = rmw_qos_profile_sensor_data;
        auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, 10), qos_profile);

        // 1. Đăng ký Subscriber
        image_sub_.subscribe(this, "image_raw", qos.get_rmw_qos_profile());
        // LƯU Ý: Topic AI của bạn có thể tên khác, hãy check 'ros2 topic list'
        // Thường là: /hobot_dnn_detection, /ai_msg_mono2d, hoặc /dnn_data
        ai_sub_.subscribe(this, "ai_msg_mono2d", qos.get_rmw_qos_profile());

        // 2. Khởi tạo Synchronizer
        sync_ = std::make_shared<message_filters::Synchronizer<SyncPolicy>>(
            SyncPolicy(10), image_sub_, ai_sub_);
        
        // 3. Đăng ký Callback
        sync_->registerCallback(std::bind(&AiDisplayNode::sync_callback, this, _1, _2));

        RCLCPP_INFO(this->get_logger(), "Display Node Started. Waiting for Sync Data...");
        
        // Tạo cửa sổ hiển thị (cho phép resize)
        cv::namedWindow("AI Custom Display", cv::WINDOW_NORMAL);
    }

private:
    void sync_callback(
        const sensor_msgs::msg::Image::ConstSharedPtr& img_msg,
        const ai_msgs::msg::PerceptionTargets::ConstSharedPtr& ai_msg) 
    {
        cv::Mat frame; // Biến ảnh cuối cùng dùng để vẽ

        // --- BƯỚC 1: CONVERT ẢNH (ĐÃ SỬA LỖI) ---
        try {
            if (img_msg->encoding == "nv12") {
                // Xử lý ảnh NV12 từ Camera RDK X5
                cv::Mat nv12_mat(img_msg->height * 3 / 2, img_msg->width, CV_8UC1, const_cast<uint8_t*>(img_msg->data.data()));
                cv::cvtColor(nv12_mat, frame, cv::COLOR_YUV2BGR_NV12);
            }
            else {
                // Xử lý các định dạng khác (bgr8, rgb8...)
                // Lấy trực tiếp .image để gán vào biến frame
                frame = cv_bridge::toCvCopy(img_msg, "bgr8")->image;
            }
        } catch (cv::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "OpenCV Error: %s", e.what());
            return;
        } catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "CV Bridge Error: %s", e.what());
            return;
        }

        if (frame.empty()) return;

        // --- BƯỚC 2: VẼ BOUNDING BOX ---
        for (const auto& target : ai_msg->targets) {
            for (const auto& roi : target.rois) {
                // Tạo hình chữ nhật
                cv::Rect rect(
                    roi.rect.x_offset, 
                    roi.rect.y_offset, 
                    roi.rect.width, 
                    roi.rect.height
                );

                // Vẽ hình chữ nhật (Màu đỏ)
                cv::rectangle(frame, rect, cv::Scalar(0, 0, 255), 2);

                // Lấy tên vật thể (Ví dụ: person, car)
                std::string label = target.type; 
                
                // Vẽ chữ (Màu xanh lá)
                cv::putText(frame, label, cv::Point(rect.x, rect.y - 10),
                            cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
            }
        }

        // --- BƯỚC 3: HIỂN THỊ ---
        // Resize nhẹ nếu ảnh quá to (tùy chọn)
        // cv::resize(frame, frame, cv::Size(960, 540));
        
        cv::imshow("AI Custom Display", frame);
        cv::waitKey(1);
    }

    message_filters::Subscriber<sensor_msgs::msg::Image> image_sub_;
    message_filters::Subscriber<ai_msgs::msg::PerceptionTargets> ai_sub_;
    
    typedef message_filters::sync_policies::ApproximateTime<
        sensor_msgs::msg::Image, 
        ai_msgs::msg::PerceptionTargets
    > SyncPolicy;
    
    std::shared_ptr<message_filters::Synchronizer<SyncPolicy>> sync_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AiDisplayNode>());
    rclcpp::shutdown();
    return 0;
}