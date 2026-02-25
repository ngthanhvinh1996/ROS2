#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <ai_msgs/msg/perception_targets.hpp> 
#include <message_filters/subscriber.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <message_filters/synchronizer.h>
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

        image_sub_.subscribe(this, "image_raw", qos.get_rmw_qos_profile());

        ai_sub_.subscribe(this, "ai_msg_mono2d", qos.get_rmw_qos_profile());

        sync_ = std::make_shared<message_filters::Synchronizer<SyncPolicy>>(
            SyncPolicy(10), image_sub_, ai_sub_);
        
        sync_->registerCallback(std::bind(&AiDisplayNode::sync_callback, this, _1, _2));

        RCLCPP_INFO(this->get_logger(), "Display Node Started. Waiting for Sync Data...");
        
        cv::namedWindow("AI Custom Display", cv::WINDOW_NORMAL);
    }

private:
    void sync_callback(
        const sensor_msgs::msg::Image::ConstSharedPtr& img_msg,
        const ai_msgs::msg::PerceptionTargets::ConstSharedPtr& ai_msg) 
    {
        cv::Mat frame;
        try {
            if (img_msg->encoding == "nv12") {
                cv::Mat nv12_mat(img_msg->height * 3 / 2, img_msg->width, CV_8UC1, const_cast<uint8_t*>(img_msg->data.data()));
                cv::cvtColor(nv12_mat, frame, cv::COLOR_YUV2BGR_NV12);
            }
            else {
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

        for (const auto& target : ai_msg->targets) {
            for (const auto& roi : target.rois) {
                cv::Rect rect(
                    roi.rect.x_offset, 
                    roi.rect.y_offset, 
                    roi.rect.width, 
                    roi.rect.height
                );

                cv::rectangle(frame, rect, cv::Scalar(0, 0, 255), 2);

                std::string label = target.type; 
                
                cv::putText(frame, label, cv::Point(rect.x, rect.y - 10),
                            cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
            }
        }
        
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