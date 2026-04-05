#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <ai_msgs/msg/perception_targets.hpp> 
#include <message_filters/subscriber.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <message_filters/synchronizer.h>
#include <opencv2/opencv.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>

using std::placeholders::_1;
using std::placeholders::_2;

typedef struct 
{
    std::string display_mode;
    std::string image_out_type;
    std::string image_sub_topic;
    std::string ai_sub_topic;
} display_node_para_st;

class AiDisplayNode : public rclcpp::Node {
public:
    AiDisplayNode() : Node("ai_display_custom_node") {
        display_node_para_ = std::make_shared<display_node_para_st>();

        display_node_para_->display_mode = this->declare_parameter<std::string>("display_mode", "single");
        display_node_para_->image_out_type = this->declare_parameter<std::string>("image_out_type", "raw");
        display_node_para_->image_sub_topic = this->declare_parameter<std::string>("image_sub_topic", "image_raw");
        display_node_para_->ai_sub_topic = this->declare_parameter<std::string>("ai_sub_topic", "ai_msg_mono2d");

        RCLCPP_INFO(this->get_logger(), "display_mode: %s" \
                                        "\n image_out_type: %s" \
                                        "\n image_sub_topic: %s" \
                                        "\n ai_sub_topic: %s" \
                                        , display_node_para_->display_mode.c_str() \
                                        , display_node_para_->image_out_type.c_str() \
                                        , display_node_para_->image_sub_topic.c_str() \
                                        , display_node_para_->ai_sub_topic.c_str());

        // QoS profile
        auto qos_profile = rclcpp::SensorDataQoS();

        image_sub_.subscribe(this, display_node_para_->image_sub_topic, qos_profile.get_rmw_qos_profile());
        pure_image_sub_ = this->create_subscription<sensor_msgs::msg::CompressedImage>(
                display_node_para_->image_sub_topic,
                10,
                [this](const sensor_msgs::msg::CompressedImage::ConstSharedPtr msg) {
                    RCLCPP_INFO_ONCE(this->get_logger(), ">>> CAPTURED TOPIC /image <<<");
                    this->latest_img_ = msg;
                }
            );

        ai_sub_.subscribe(this, display_node_para_->ai_sub_topic, qos_profile.get_rmw_qos_profile());

        if(0 == display_node_para_->display_mode.compare("combine"))
        {
            RCLCPP_INFO_ONCE(this->get_logger(), "Run combine mode...");
            sync_ = std::make_shared<message_filters::Synchronizer<SyncPolicy>>(
                SyncPolicy(10), image_sub_, ai_sub_);
            
            sync_->registerCallback(std::bind(&AiDisplayNode::sync_callback, this, _1, _2));
        }
        else
        {
            RCLCPP_INFO_ONCE(this->get_logger(), "Run single mode...");
            image_sub_.registerCallback([this](const sensor_msgs::msg::CompressedImage::ConstSharedPtr& msg) {
                RCLCPP_INFO_ONCE(this->get_logger(), "Get image msg...");
                this->latest_img_ = msg;
            });

            ai_sub_.registerCallback([this](const ai_msgs::msg::PerceptionTargets::ConstSharedPtr& msg) {
                this->latest_ai_ = msg;
            });

            timer_ = this->create_wall_timer(
                std::chrono::milliseconds(33),
                [this]() {
                    if (this->latest_img_ != nullptr) {
                        this->sync_callback(this->latest_img_, this->latest_ai_);

                        this->latest_img_ = nullptr; 
                    }
                }
            );
        }

        RCLCPP_INFO(this->get_logger(), "Display Node Started. Waiting for Sync Data...");
        
        cv::namedWindow("AI Custom Display", cv::WINDOW_NORMAL);
    }

private:
    void sync_callback(
        const sensor_msgs::msg::CompressedImage::ConstSharedPtr& img_msg,
        const ai_msgs::msg::PerceptionTargets::ConstSharedPtr& ai_msg) 
    {
        cv::Mat frame;

        try {
            if(0 == display_node_para_->image_out_type.compare("jpeg"))
            {
                RCLCPP_INFO_ONCE(this->get_logger(), "Get image type jpeg");
                cv::Mat raw_data(1, img_msg->data.size(), CV_8UC1, (void*)img_msg->data.data());

                frame = cv::imdecode(raw_data, cv::IMREAD_COLOR);

                if (frame.empty()) {
                    return; 
                }
            }
            
        } catch (cv::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "OpenCV Error: %s", e.what());
            return;
        }
        
        if (frame.empty()) {
            return; 
        }

        if(0 == display_node_para_->display_mode.compare("combine"))
        {
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
        }
        
        cv::imshow("AI Custom Display", frame);
        cv::waitKey(1);
    }

    message_filters::Subscriber<sensor_msgs::msg::CompressedImage> image_sub_;
    rclcpp::Subscription<sensor_msgs::msg::CompressedImage>::SharedPtr pure_image_sub_;
    message_filters::Subscriber<ai_msgs::msg::PerceptionTargets> ai_sub_;
    
    typedef message_filters::sync_policies::ApproximateTime<
        sensor_msgs::msg::CompressedImage, 
        ai_msgs::msg::PerceptionTargets
    > SyncPolicy;
    
    std::shared_ptr<message_filters::Synchronizer<SyncPolicy>> sync_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::shared_ptr<display_node_para_st> display_node_para_;
    sensor_msgs::msg::CompressedImage::ConstSharedPtr latest_img_ = nullptr;
    ai_msgs::msg::PerceptionTargets::ConstSharedPtr latest_ai_ = nullptr;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AiDisplayNode>());
    rclcpp::shutdown();
    return 0;
}