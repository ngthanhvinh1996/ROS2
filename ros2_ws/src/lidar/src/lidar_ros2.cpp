#include "lidar_ros2.hpp"

namespace lidar {

LidarRos2::LidarRos2(const rclcpp::NodeOptions& options) : Node("lidar_node", options)
{
    Logger::setInfoCallback([this](const std::string& msg) { RCLCPP_INFO(this->get_logger(), "%s", msg.c_str()); });
    Logger::setWarnCallback([this](const std::string& msg) { RCLCPP_WARN(this->get_logger(), "%s", msg.c_str()); });
    Logger::setErrorCallback([this](const std::string& msg) { RCLCPP_ERROR(this->get_logger(), "%s", msg.c_str()); });

    load_parameters();

    driver_ = std::make_unique<LidarDriver>(config_.port_name, config_.baudrate);

    if(!driver_->open())
    {
        RCLCPP_ERROR(this->get_logger(), "Failed to open port lidar %s", config_.port_name.c_str());
        return;
    }

    publisher_ = this->create_publisher<sensor_msgs::msg::LaserScan>(config_.scan_topic, 10);

    first_time_flag_ = true;

    RCLCPP_INFO(this->get_logger(), "Lidar node started on %s at %s", config_.frame_id.c_str(), config_.scan_topic.c_str());
}

LidarRos2::~LidarRos2()
{

}

void LidarRos2::load_parameters()
{
    config_.port_name = std::string("/dev/ttyUSB0");
    config_.baudrate = 460800U;
    config_.frame_id = std::string("laser");
    config_.scan_topic = std::string("scan");
    config_.min_range = 0.2f;
    config_.max_range = 200.0f;
    config_.valid_point = 16U;
    
    this->declare_parameter<std::string>("port_name", config_.port_name);
    this->declare_parameter<int>("baud_rate", config_.baudrate);
    this->declare_parameter<std::string>("frame_id", config_.frame_id);
    this->declare_parameter<std::string>("scan_topic", config_.scan_topic);
    this->declare_parameter<float>("min_range", config_.min_range);
    this->declare_parameter<float>("max_range", config_.max_range);
    this->declare_parameter<uint8_t>("valid_point", config_.valid_point);

    this->get_parameter("port_name", config_.port_name);
    this->get_parameter("baud_rate", config_.baudrate);
    this->get_parameter("frame_id", config_.frame_id);
    this->get_parameter("scan_topic", config_.scan_topic);
    this->get_parameter("min_range", config_.min_range);
    this->get_parameter("max_range", config_.max_range);
    this->get_parameter("valid_point", config_.valid_point);

    RCLCPP_INFO(this->get_logger(), "\n Port name: %s" \
                                    "\n Baud rate: %d" \
                                    "\n Frame id: %s" \
                                    "\n Scan topic: %s" \
                                    "\n Min range: %f" \
                                    "\n Max range: %f" \
                                    "\n Valid point: %d", 
                                    config_.port_name.c_str(), 
                                    config_.baudrate, 
                                    config_.frame_id.c_str(), 
                                    config_.scan_topic.c_str(), 
                                    config_.min_range, 
                                    config_.max_range,
                                    config_.valid_point);

    // Allocate buffer for one full lidar packet (108 bytes)
    data_ = (unsigned char*)calloc(PACKET_SIZE, sizeof(unsigned char));

    scan_data_.resize(PACKET_SIZE * 10);
    scan_data_processing_.resize(PACKET_SIZE * 10);

    idx_ = 0U;
    idx_processing_ = 0U;
}

uint8_t LidarRos2::calculate_crc8(const unsigned char *data, size_t len)
{
    uint8_t crc = 0;
    int sum = 0;
    for(size_t i = 0; i < len; i++)
    {
        sum  += data[i];
    }
    crc = sum & 0xff;
    return crc;
}

bool LidarRos2::feed_data(unsigned char *data, size_t len)
{
    ssize_t bytes_read;
    uint8_t count = 0U;

    do {
        bytes_read = driver_->read_bytes(data + count, 1);
        
        if(0 > bytes_read)
        {
            RCLCPP_ERROR(this->get_logger(), "Get PACKET_HEADER_BYTE_0 error");
            return false;
        }
    } while(PACKET_HEADER_BYTE_0 != data[count]);

    count++;

    bytes_read = driver_->read_bytes(data + count, 1);
    if(PACKET_HEADER_BYTE_1 != data[count])
    {
        RCLCPP_ERROR(this->get_logger(), "Get PACKET_HEADER_BYTE_1 error");
        return false;
    }

    count++;

    while(count < len)
    {
        bytes_read = driver_->read_bytes(data + count, len - count);
        if(0 < bytes_read)
        {
            count += bytes_read;
        }
    }

    if(data[len - 1] != calculate_crc8(data, (len - 1)))
    {
        RCLCPP_ERROR(this->get_logger(), "CRC8 error");
        return false;
    }

    return true;
}

void LidarRos2::parse_packet(unsigned char *data)
{
    float start_angle;
    float stop_angle;
    float angle_increment;
    float angle_interval;

    if((PACKET_HEADER_BYTE_0 == data[0]) && (PACKET_HEADER_BYTE_1 == data[1]))
    {
        start_angle = (data[5] * 256 + data[6]) / 100.0f;
        start_angle = (start_angle > 360.0f) ? start_angle - 360.0f : start_angle;
        stop_angle = (data[105] * 256 + data[106]) / 100.0f;
        stop_angle = (stop_angle > 360.0f) ? stop_angle - 360.0f : stop_angle;
        if(start_angle < stop_angle)
        {
            angle_interval = (stop_angle - start_angle);
        }
        else
        {
            angle_interval = (360.0f + stop_angle - start_angle);
        }
        
        angle_increment = angle_interval / (config_.valid_point - 1);

        for(int i = 0; i < config_.valid_point; i++)
        {
            scan_data_[idx_].angle = start_angle + i * angle_increment;
            if(360.0f < scan_data_[idx_].angle)
            {
                scan_data_[idx_].angle = scan_data_[idx_].angle - 360.0f;
            }
            scan_data_[idx_].range = (data[7 + i * 6] * 256 + data[8 + i * 6]) / 1000.0f;
            if((scan_data_[idx_].range < config_.min_range) || (scan_data_[idx_].range > config_.max_range))
            {
                scan_data_[idx_].range = 0.0f;
            }
            scan_data_[idx_].intensity = data[9 + i * 6];
            if((idx_ > 0) && (scan_data_[idx_].angle < scan_data_[idx_ - 1].angle))
            {
                float angle;
                float range;
                float intensity;

                scan_data_processing_ = scan_data_;
                idx_processing_ = idx_ - 1;
                angle = scan_data_[idx_].angle;
                range = scan_data_[idx_].range;
                intensity = scan_data_[idx_].intensity;
                idx_ = 0U;

                scan_data_[idx_].angle = angle;
                scan_data_[idx_].range = range;
                scan_data_[idx_].intensity = intensity;
                idx_++;
            }
            else
            {
                idx_++;
                if((PACKET_SIZE * 10) == idx_)
                {
                    idx_ = 0U;
                }
            }
        }
    }
}

bool LidarRos2::processing_lidar()
{
    if(true == feed_data(data_, PACKET_SIZE))
    {
        parse_packet(data_);

        if(0 != idx_processing_)
        {
            auto scan = sensor_msgs::msg::LaserScan::UniquePtr(new sensor_msgs::msg::LaserScan());

            scan->header.frame_id = config_.frame_id;
            scan->header.stamp = this->now();

            scan->angle_min = 0.0f;
            scan->angle_max = 2 * M_PI;
            scan->angle_increment = 2 * M_PI / (double)(idx_processing_ + 1);
            scan->range_min = config_.min_range;
            scan->range_max = config_.max_range;
            scan->ranges.reserve(idx_processing_ + 1);
            scan->ranges.assign(idx_processing_ + 1, std::numeric_limits<float>::infinity());
            scan->intensities.reserve(idx_processing_ + 1);
            scan->intensities.assign(idx_processing_ + 1, std::numeric_limits<float>::infinity());
            
            for(uint32_t i = 0; i <= idx_processing_; i++)
            {
                scan->ranges[i] = std::numeric_limits<float>::infinity();
                scan->intensities[i] = 0.0f;
            }

            for(uint32_t i = 0; i <= idx_processing_; i++)
            {
                if (scan_data_processing_[i].range == 0.0)
                {
                    scan->ranges[i] = std::numeric_limits<float>::infinity();
                    scan->intensities[i] = 0;
                }
                else
                {
                    scan->ranges[i] = scan_data_processing_[i].range;
                    scan->intensities[i] = scan_data_processing_[i].intensity;
                }
            }
            
            publisher_->publish(std::move(scan));

            idx_processing_ = 0;
        }
    }

    return true;
}

}
