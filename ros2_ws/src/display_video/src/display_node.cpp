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

typedef struct 
{
    std::string image_out_type;
    std::string image_sub_topic;
    std::string ai_sub_topic;
} display_node_para_st;

class AiDisplayNode : public rclcpp::Node {
public:
    AiDisplayNode() : Node("ai_display_custom_node") {
        display_node_para_ = std::make_shared<display_node_para_st>();

        display_node_para_->image_out_type = "raw";
        display_node_para_->image_sub_topic = "image_raw";
        display_node_para_->ai_sub_topic = "ai_msg_mono2d";

        this->declare_parameter<std::string>("image_out_type", display_node_para_->image_out_type);
        this->declare_parameter<std::string>("image_sub_topic", display_node_para_->image_sub_topic);
        this->declare_parameter<std::string>("ai_sub_topic", display_node_para_->ai_sub_topic);

        this->get_parameter("image_out_type", display_node_para_->image_out_type);
        this->get_parameter("image_sub_topic", display_node_para_->image_sub_topic);
        this->get_parameter("ai_sub_topic", display_node_para_->ai_sub_topic);

        RCLCPP_INFO(this->get_logger(), "image_out_type: %s" \
                                        "\n image_sub_topic: %s" \
                                        "\n ai_sub_topic: %s" \
                                        , display_node_para_->image_out_type.c_str() \
                                        , display_node_para_->image_sub_topic.c_str() \
                                        , display_node_para_->ai_sub_topic.c_str());

        // QoS profile
        rmw_qos_profile_t qos_profile = rmw_qos_profile_sensor_data;
        auto qos = rclcpp::QoS(rclcpp::QoSInitialization(qos_profile.history, 10), qos_profile);

        image_sub_.subscribe(this, display_node_para_->image_sub_topic, qos.get_rmw_qos_profile());

        ai_sub_.subscribe(this, display_node_para_->ai_sub_topic, qos.get_rmw_qos_profile());

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
            if(0 == display_node_para_->image_out_type.compare("raw"))
            {
                if (img_msg->encoding == "nv12") {
                    cv::Mat nv12_mat(img_msg->height * 3 / 2, img_msg->width, CV_8UC1, const_cast<uint8_t*>(img_msg->data.data()));
                    cv::cvtColor(nv12_mat, frame, cv::COLOR_YUV2BGR_NV12);
                }
                else {
                    frame = cv_bridge::toCvCopy(img_msg, "bgr8")->image;
                }
            }
            else if(0 == display_node_para_->image_out_type.compare("jpeg"))
            {
                std::vector<uint8_t> img_data(img_msg->data.begin(), img_msg->data.end());

                frame = cv::imdecode(img_data, cv::IMREAD_COLOR);
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

                cv::rectangle(frame, rect, cv::Scalar(0, 255, 0), 2);

                std::string label = target.type; 
                
                cv::putText(frame, label, cv::Point(rect.x, rect.y - 5),
                            cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
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
    std::shared_ptr<display_node_para_st> display_node_para_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AiDisplayNode>());
    rclcpp::shutdown();
    return 0;
}