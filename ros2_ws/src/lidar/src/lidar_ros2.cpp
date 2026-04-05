#include "lidar_ros2.hpp"

namespace lidar {

LidarRos2::LidarRos2(const rclcpp::NodeOptions& options) : Node("lidar_node", options)
{
    Logger::setInfoCallback([this](const std::string& msg) { RCLCPP_INFO(this->get_logger(), "%s", msg.c_str()); });
    Logger::setWarnCallback([this](const std::string& msg) { RCLCPP_WARN(this->get_logger(), "%s", msg.c_str()); });
    Logger::setErrorCallback([this](const std::string& msg) { RCLCPP_ERROR(this->get_logger(), "%s", msg.c_str()); });

    // Allocate buffer for one full lidar packet (108 bytes)
    data_.resize(108);

    load_parameters();

    driver_ = std::make_unique<LidarDriver>(config_.port_name, config_.baudrate);

    if(!driver_->open())
    {
        RCLCPP_ERROR(this->get_logger(), "Failed to open port lidar %s", config_.port_name.c_str());
        return;
    }

    publisher_ = this->create_publisher<sensor_msgs::msg::LaserScan>(config_.scan_topic, 10);

    timer_ = this->create_wall_timer(std::chrono::milliseconds(100), std::bind(&LidarRos2::timer_callback, this));

    RCLCPP_INFO(this->get_logger(), "Lidar node started on %s at %s", config_.frame_id.c_str(), config_.scan_topic.c_str());
}

LidarRos2::~LidarRos2()
{

}

void LidarRos2::timer_callback()
{
    feed_data(data_, 108);
}

void LidarRos2::publish_data(const ScanData& data)
{

}

void LidarRos2::load_parameters()
{
    config_.port_name = std::string("/dev/ttyUSB0");
    config_.baudrate = 460800U;
    config_.frame_id = std::string("laser_link");
    config_.scan_topic = std::string("scan");
    
    this->declare_parameter<std::string>("port_name", config_.port_name);
    this->declare_parameter<int>("baud_rate", config_.baudrate);
    this->declare_parameter<std::string>("frame_id", config_.frame_id);
    this->declare_parameter<std::string>("scan_topic", config_.scan_topic);

    this->get_parameter("port_name", config_.port_name);
    this->get_parameter("baud_rate", config_.baudrate);
    this->get_parameter("frame_id", config_.frame_id);
    this->get_parameter("scan_topic", config_.scan_topic);

    RCLCPP_INFO(this->get_logger(), "Port name: %s", config_.port_name.c_str());
    RCLCPP_INFO(this->get_logger(), "Baud rate: %d", config_.baudrate);
    RCLCPP_INFO(this->get_logger(), "Frame id: %s", config_.frame_id.c_str());
    RCLCPP_INFO(this->get_logger(), "Scan topic: %s", config_.scan_topic.c_str());
}

void LidarRos2::feed_data(std::vector<uint8_t>& data, size_t len)
{
    ssize_t bytes_read;
    uint8_t index = 0U;

    bytes_read = driver_->read_bytes(data, 0, 1);
    if(0 > bytes_read)
    {
        return;
    }

    if(0xA5 != data[0])
    {
        return;
    }

    RCLCPP_INFO(this->get_logger(), "HEADER: 0x%x", data[0]);

    bytes_read = driver_->read_bytes(data, 1, 1);
    if(0 > bytes_read)
    {
        return;
    }

    if(0x5A != data[1])
    {
        return;
    }

    RCLCPP_INFO(this->get_logger(), "HEADER: 0x%x", data[1]);

    for(index = 0; index < len - 5; index++)
    {
        bytes_read = driver_->read_bytes(data, index + 5, 1);
        if(0 > bytes_read)
        {
            return;
        }
    }

    for(index = 0; index < len; index++)
    {
        RCLCPP_INFO(this->get_logger(), "Data: 0x%x", data[index]);
    }
    
}

void LidarRos2::parse_packet(const std::vector<uint8_t>& data, size_t len)
{
    // RCLCPP_INFO(this->get_logger(), "HEADER: 0x%x 0x%x", data[0], data[1]);
    // if((0xA5 == data[0]) && (0x5A == data[1]))
    // {
    //     scan_data_.angle_min = (data[5] * 256 + data[6]) / 100.0f;
    //     scan_data_.angle_max = (data[105] * 256 + data[106]) / 100.0f;
    //     scan_data_.angle_increment = (scan_data_.angle_max - scan_data_.angle_min) / 16.0f;
    //     RCLCPP_INFO(this->get_logger(), "Angle_min: %.2f, Angle_max: %.2f, increment: %.2f", scan_data_.angle_min, scan_data_.angle_max, scan_data_.angle_increment);
    //     scan_data_.ranges.resize(16);
    //     scan_data_.intensities.resize(16);
    //     for(int i = 0; i < 16; i++)
    //     {
    //         scan_data_.ranges[i] = (data[7 + i * 6] * 256 + data[8 + i * 6]) / 1000.0f;
    //         scan_data_.intensities[i] = data[9 + i * 6];
    //         RCLCPP_INFO(this->get_logger(), "Range: %.2f, Intensity: %.2f", scan_data_.ranges[i], scan_data_.intensities[i]);
    //     }
    // }
}

}
