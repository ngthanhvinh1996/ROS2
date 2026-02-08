#include "robot_camera_node.hpp"

#define PUB_BUF_NUM 5

namespace robot_cam
{

RobotCameraNode::RobotCameraNode(const rclcpp::NodeOptions &node_options)
    : m_bIsInit(0),
    Node("robot_cam", node_options),
    camera_calibration_info_(new sensor_msgs::msg::CameraInfo())
{
    nodePara_ = std::make_shared<struct NodePara>();

    std::string tros_distro = std::string(std::getenv("TROS_DISTRO") ? std::getenv("TROS_DISTRO") :: "");
    nodePara_->config_path_ = "opt/tros/" + tros_distro + "/lib/robot_cam/config";
    nodePara_->video_device_name_ = "";
    nodePara_->channel_ = 0;
    nodePara_->channel2_ = 2;
    nodePara_->camera_info_url_ = "";
    nodePara_->camera_calibration_file_path_ = "";
    nodePara_->out_format_name_ = "bgr8";
    nodePara_->gdc_bin_file_ = "";
    nodePara_->image_width_ = 1920;
    nodePara_->image_height_ = 1080;
    nodePara_->sub_image_width_ = 960;
    nodePara_->sub_image_height_ = 540;
    nodePara_->framerate_ = 30;
    nodePara_->rotation_ = 0.0;
    nodePara_->cal_rotation_ = 0.0;
    nodePara_->device_mode_ = "single";
    nodePara_->dual_combine_ = 0;
    nodePara_->lpwm_enable_ = false;
    nodePara_->gdc_enable_ = true;
    nodePara_->frame_ts_type_ = "sensor";
    nodePara_->link_type_ = 0;
    nodePara_->link_port_ = 0;
    nodePara_->cal_alpha_ = 0.0;
    nodePara_->sub_stream_flag_ = false;
    frame_id_ = "default_cam";
    io_method_name_ = "ros";

    double framerate = 30.0;

    this->declare_parameter<std::string>("frame_id", frame_id_);
    this->declare_parameter<std::string>("io_method", io_method_name_);
    this->declare_parameter<std::string>("config_path", nodePara_->config_path_);
    this->declare_parameter<std::string>("video_device", nodePara_->video_device_name_);
    this->declare_parameter<int>("channel", nodePara_->channel_);
    this->declare_parameter<int>("channel2", nodePara_->channel2_);
    this->declare_parameter<std::string>("camera_info_url", nodePara_->camera_info_url_);
    this->declare_parameter<std::string>("camera_calibration_file_path", nodePara_->camera_calibration_file_path_);
    this->declare_parameter<std::string>("out_format", nodePara_->out_format_name_);
    this->declare_parameter<std::string>("gdc_bin_file", nodePara_->gdc_bin_file_);
    this->declare_parameter<int>("image_width", nodePara_->image_width_);
    this->declare_parameter<int>("image_height", nodePara_->image_height_);
    this->declare_parameter<int>("sub_image_width", nodePara_->sub_image_width_);
    this->declare_parameter<int>("sub_image_height", nodePara_->sub_image_height_);
    this->declare_parameter<double>("framerate", framerate);
    this->declare_parameter<double>("rotation", nodePara_->rotation_);
    this->declare_parameter<double>("cal_rotation", nodePara_->cal_rotation_);
    this->declare_parameter<std::string>("device_mode", nodePara_->device_mode_);  
    this->declare_parameter<int>("dual_combine", nodePara_->dual_combine_);
    this->declare_parameter<bool>("lpwm_enable", nodePara_->lpwm_enable_);
    this->declare_parameter<bool>("gdc_enable", nodePara_->gdc_enable_);
    this->declare_parameter<std::string>("frame_ts_type", nodePara_->frame_ts_type_);
    this->declare_parameter<int>("link_type", nodePara_->link_type_);
    this->declare_parameter<int>("link_port", nodePara_->link_port_);
    this->declare_parameter<double>("cal_alpha", nodePara_->cal_alpha_);

    this->get_parameter<std::string>("frame_id", frame_id_);
    this->get_parameter<std::string>("io_method", io_method_name_); 
    this->get_parameter<std::string>("config_path", nodePara_->config_path_);
    this->get_parameter<std::string>("video_device", nodePara_->video_device_name_);
    this->get_parameter<int>("channel", nodePara_->channel_);
    this->get_parameter<int>("channel2", nodePara_->channel2_);
    this->get_parameter<std::string>("camera_info_url", nodePara_->camera_info_url_);
    this->get_parameter<std::string>("camera_calibration_file_path", nodePara_->camera_calibration_file_path_);
    this->get_parameter<std::string>("out_format", nodePara_->out_format_name_); 
    this->get_parameter<std::string>("gdc_bin_file", nodePara_->gdc_bin_file_);
    this->get_parameter<int>("image_width", nodePara_->image_width_);
    this->get_parameter<int>("image_height", nodePara_->image_height_);
    this->get_parameter<int>("sub_image_width", nodePara_->sub_image_width_);
    this->get_parameter<int>("sub_image_height", nodePara_->sub_image_height_);
    this->get_parameter<double>("framerate", framerate);
    this->get_parameter<double>("rotation", nodePara_->rotation_);
    this->get_parameter<double>("cal_rotation", nodePara_->cal_rotation_);
    this->get_parameter<std::string>("device_mode", nodePara_->device_mode_);  
    this->get_parameter<int>("dual_combine", nodePara_->dual_combine_);
    this->get_parameter<bool>("lpwm_enable", nodePara_->lpwm_enable_);
    this->get_parameter<bool>("gdc_enable", nodePara_->gdc_enable_);
    this->get_parameter<std::string>("frame_ts_type", nodePara_->frame_ts_type_);
    this->get_parameter<int>("link_type", nodePara_->link_type_);
    this->get_parameter<int>("link_port", nodePara_->link_port_);
    this->get_parameter<double>("cal_alpha", nodePara_->cal_alpha_);

    nodePara_->framerate_ = static_cast<int>(framerate);

    RCLCPP_INFO(rclcpp::get_logger("robot_cam"),
        "\n node params: " \
        "\n config_path: %s" \
        "\n video_device_name: %s" \
        "\n channel: %d" \
        "\n channel2: %d" \
        "\n camera_info_url: %s" \
        "\n camera_calibration_file_path: %s" \
        "\n              out_format_name: %s" \
        "\n                 gdc_bin_file: %s" \
        "\n                  image_width: %d" \
        "\n                 image_height: %d" \
        "\n               sub_image_width: %d" \
        "\n              sub_image_height: %d" \
        "\n                    framerate: %d" \
        "\n                     rotation: %f" \
        "\n                  device_mode: %s" \
        "\n                 dual_combine: %d" \
        "\n                  lpwm_enable: %s" \
        "\n                   gdc_enable: %s" \
        "\n                frame_ts_type: %s" \
        "\n                     frame_id: %s" \
        "\n                    link_type: %d" \
        "\n                    link_port: %d" \
        "\n               io_method_name: %s" \
        "\n                    cal_alpha: %.3f",
        nodePara_->config_path_.c_str(),
        nodePara_->video_device_name_.c_str(),
        nodePara_->channel_,
        nodePara_->channel2_,
        nodePara_->camera_info_url_.c_str(),
        nodePara_->camera_calibration_file_path_.c_str(),
        nodePara_->out_format_name_.c_str(),
        nodePara_->gdc_bin_file_.c_str(),
        nodePara_->image_width_,
        nodePara_->image_height_,
        nodePara_->sub_image_width_,
        nodePara_->sub_image_height_,
        nodePara_->framerate_,
        nodePara_->rotation_,
        nodePara_->device_mode_.c_str(),
        nodePara_->dual_combine_,
        (nodePara_->lpwm_enable_ ? "true" : "false"),
        (nodePara_->gdc_enable_ ? "true" : "false"),
        nodePara_->frame_ts_type_.c_str(),
        frame_id_.c_str(),
        nodePara_->link_type_,
        nodePara_->link_port_,
        io_method_name_.c_str(),
        nodePara_->cal_alpha_
    );

    init();
}

RobotCameraNode::~RobotCameraNode()
{
    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "shutting down");

    for(auto timer : timer_)
    {
        timer->join();
    }
    timer_.clear();

    if(robotCam_ptr_)
    {
        robotCam_ptr_->stop();
        robotCam_ptr_->deInit();
        RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "shutting down end");
    }

    for(auto &pub : Pub_info_)
    {
        pub.image_pub_.reset();
        pub.info_pub_.reset();
        pub.info_pub2_.reset();
    }
}

void RobotCameraNode::init()
{
    if(m_bIsInit)
    {
        return;
    }

    robotCam_ptr_ = MipiCam::create_camera();
    if(!robotCam_ptr_ || robotCam_ptr_->init(nodePara_))
    {
        RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "[%s]->robotnode init failure.\n", __func__);
        rclcpp::shutdown();
    }

    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "[RobotCamNode::%s]-> Initing '%s' at %dx%d via %s at %i FPS",
            __func__,
            nodePara_->config_path_.c_str(),
            nodePara_->image_width_,
            nodePara_->image_height_,
            io_method_name_.c_str(),
            nodePara_->framerate_);
    
    if(0 == io_method_name_.compare("ros"))
    {
        if(0 == nodePara_->device_mode_.compare("dual"))
        {
            if(1 == nodePara_->dual_combine_)
            {
                Pub_info_.resize(3);
                init_DualCalibration(&Pub_info_[0], &Pub_info_[1], "/image_left_raw/camera_info", "/image_right_raw/camera_info", nodePara_->camera_calibration_file_path_);
                init_publisher(Pub_info[0], "image_left_raw", "left", frame_id_);
                init_publisher(Pub_info[1], "image_right_raw", "right", frame_id_);
                init_publisher(Pub_info[2], "image_combine_raw", "combine", frame_id_);
            }
            else if(2 == nodePara_->dual_combine_)
            {
                Pub_info_.resize(1);
                init_DualCalibration(&Pub_info_[0], "/image_left_raw/camera_info", "image_right_raw/camera_info", nodePara_->camera_calibration_file_path_);
                init_publisher(Pub_info[0], "image_combine_raw", "combine", frame_id_);
            }
            else
            {
                Pub_info_.resize(2);
                init_DualCalibration(&Pub_info_[0], &Pub_info_[1], "image_left_raw/camera_info", "image_right_raw/camera_info", nodePara_->camera_calibration_file+path_);
                init_publisher(Pub_info[0], "image_left_raw", "left", frame_id_);
                init_publisher(Pub_info[1], "image_right_raw", "right", frame_id_);
            }
        }
        else if((0 == nodePara_->device_mode_.compare("single")) || (0 == nodePara_->device_mode_.compare("")))
        {
            if(nodePara_->sub_stream_flag_)
            {
                Pub_info_.resize(2);
                init_Calibration(&Pub_info_[0], "image_raw/camera_info", nodePara_->camera_calibration_file_path_);
                init_publisher(Pub_info_[0], "image_raw", "single", frame_id_);
                init_Calibration(&Pub_info_[1], "sub_image_raw/camera_info", nodePara_->camera_calibration_file_path_);
                init_publisher(Pub_info_[1], "sub_image_raw", "sub_single", frame_id_);
            }
            else
            {
                Pub_info_.resize(1);
                init_Calibration(&Pub_info_[0], "image_raw/camera_info", nodePara_->camera_calibration_file_path_);
                init_publisher(Pub_info_[0], "image_raw", "single", frame_id_);
            }
        }
        esle
        {
            return;
        }
    }

    /* Start the camera */
    if(0 != robotCam_ptr_->start())
    {
        RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "robot camera start failed!");
        rclcpp::shutdown();
        return;
    }

    const int period_ms = 1000.0 / nodePara_->framerate_;

    if(0 == io_method_name_.compare("ros"))
    {
        for(Publisher_info_st &info : Pub_info)
        {
            timer_.emplace_back(std::make_shared<std::thread>([this, &info]() {while(rclcpp::ok()) {this->update(&info);}}))
        }
    }

    RCLCPP_INFO_STREAM(rclcpp::get_logger("robot_node"), "starting timer " << period_ms);

    m_bIsInit = 1;
}

void RobotCameraNode::init_publisher(Publisher_info_st &Pub_info, std::string topic, std::string topic_type, std::string frame_id)
{
    Pub_info.image_pub_ = this->create_publisher<sensor_msgs::msg::Image>(topic, PUB_BUF_NUM);
    Pub_info.img_ = std::make_unique<sensor_msgs::msg::Image>(rosidl_runtime_cpp::MessageInitialization::SKIP);

    Pub_info.img_->header.frame_id = frame_id;
    Pub_info.topic_type = topic_type;
    Pub_info.time_start_ = std::chrono::system_clock::now();
}

void RobotCameraNode::update(Publisher_info_st* pub_info)
{
    if(robotCam_ptr_ && robotCam_ptr_->isCapturing())
    {
        if(!robotCam_ptr_->getImage(pub_info->img_->header.stamp,
                                    pub_info->img_->encoding,
                                    pub_info->img_->height,
                                    pub_info->img_->width,
                                    pub_info->img_->step,
                                    pub_info->img_->data,
                                    pub_info->topic_type)) 
        {
            auto timer_after = std::chrono::system_clock::now();
            auto interval = std::chrono::duration_cast<std::chrono::miliseconds>(timer_after - pub_info->timer_start_).count();
            if(interval > 3000)
            {
                RCLCPP_ERROR(rclcpp::get_logger("robot_node"), "grab failed.");
                return;
            }
        }

        pub_info->image_pub_->publish(std::move(pub_info->img_));
        pub_info->img_ = std::make_unique<sensor_msgs::msg::Image>(rosidl_runtime_cpp::MessageInitialization::SKIP);

        if(pub_info->info_pub_)
        {
            pub_info->camera_calibration_info_->header.stamp = pub_info->img_->header.stamp;
            pub_info->info_pub_->publish(*pub_info->camera_calibration_info_);
        }

        if(pub_info->info_pub2_)
        {
            pub_info->camera_calibration_info2_->header.stamp = pub_info->img_->header.stamp;
            pub_info->info_pub2_->publish(*pub_info->camera_calibration_info2_);
        }
    }
}

} // robot_cam
