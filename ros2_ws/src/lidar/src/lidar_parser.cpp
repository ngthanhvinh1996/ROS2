#include "lidar_parser.hpp"

namespace lidar {

LidarParser::LidarParser()
{
    state_ = ParseState::WAIT_HEADER;
    scan_ready_ = false;
    buffer_.reserve(1024);
}

LidarParser::~LidarParser()
{
}

void LidarParser::feed_bytes(const std::vector<uint8_t> &data, size_t len)
{
    uint32_t length = 0U;

    for(size_t i = 0; i < len; i++)
    {
        buffer_.emplace_back(data[i]);
        switch(state_)
        {
            case ParseState::WAIT_HEADER:
                if(2 > buffer_.size())
                {
                    break;
                }
                else if((0xA5 == buffer_[0]) && (0x5A == buffer_[1]))
                {
                    state_ = ParseState::WAIT_LENGTH;
                    LOG_INFO("[%s][%s][%d] Found header", __FILE__, __FUNCTION__, __LINE__);
                }
                else
                {
                    LOG_WARN("[%s][%s][%d] Not found header", __FILE__, __FUNCTION__, __LINE__);
                    buffer_.clear();
                }
                break;
            case ParseState::WAIT_LENGTH:
                if(5 <= buffer_.size())
                {
                    /* The length of packet have 3 bytes */
                    length = (buffer_[2] | (buffer_[3] << 8U) | (buffer_[4] << 16U)) & 0xFFFF;
                    if((102 <= length) && (108 >= length))
                    {
                        state_ = ParseState::WAIT_DATA;
                        LOG_INFO("[%s][%s][%d] Found length: %d", __FILE__, __FUNCTION__, __LINE__, length);
                    }
                    else
                    {
                        LOG_WARN("[%s][%s][%d] Not found length: %d", __FILE__, __FUNCTION__, __LINE__, length);
                        buffer_.clear();
                        state_ = ParseState::WAIT_HEADER;
                    }
                }
                break;
            case ParseState::WAIT_DATA:
                if((buffer_.size() > LIDAR_HEADER_LENGTH) && ((length - 1) <= buffer_.size()))
                {
                    state_ = ParseState::WAIT_CHECKSUM;
                    LOG_INFO("[%s][%s][%d] Found data", __FILE__, __FUNCTION__, __LINE__);
                }
                break;
            case ParseState::WAIT_CHECKSUM:
                if(verify_checksum(buffer_, buffer_.size()))
                {
                    parse_packet(buffer_, buffer_.size());
                    LOG_INFO("[%s][%s][%d] Found checksum", __FILE__, __FUNCTION__, __LINE__);
                }
                else
                {
                    LOG_WARN("[%s][%s][%d] Not found checksum", __FILE__, __FUNCTION__, __LINE__);
                    
                }
                buffer_.clear();
                state_ = ParseState::WAIT_HEADER;
                break;
            default:
                LOG_WARN("[%s][%s][%d] Invalid state", __FILE__, __FUNCTION__, __LINE__);
                buffer_.clear();
                state_ = ParseState::WAIT_HEADER;
                break;
        }
    }
}

ScanData LidarParser::get_scan()
{
    scan_ready_ = false;
    return current_scan_;
}

bool LidarParser::has_complete_scan() const
{
    return scan_ready_;
}

void LidarParser::reset()
{
    buffer_.clear();
    state_ = ParseState::WAIT_HEADER;
    scan_ready_ = false;
    LOG_INFO("[%s][%s][%d] Reset parser", __FILE__, __FUNCTION__, __LINE__);
}

bool LidarParser::verify_checksum(const std::vector<uint8_t> &packet, size_t len)
{
    uint8_t checksum = 0;
    for(size_t i = 0; i < len - 1; i++)
    {
        checksum += packet[i];
    }
    return checksum == packet[len - 1];
}

void LidarParser::parse_packet(const std::vector<uint8_t> &packet, size_t len)
{
    if(LIDAR_DEGREE_BIT_END > len)
    {
        LOG_ERROR("[%s][%s][%d] Length packet wrong: %d", __FILE__, __FUNCTION__, __LINE__, len);
        return;
    }

    current_scan_.angle_min = (packet[LIDAR_DEGREE_BIT_START] * 256 + packet[LIDAR_DEGREE_BIT_START + 1]) / 100.0;
    current_scan_.angle_max = (packet[LIDAR_DEGREE_BIT_END] * 256 + packet[LIDAR_DEGREE_BIT_END + 1]) / 100.0;
    current_scan_.angle_increment = (current_scan_.angle_max - current_scan_.angle_min) / 15.0;
    current_scan_.ranges.resize(16);
    current_scan_.intensities.resize(16);

    for(size_t i = 0; i < 16; i++)
    {
        uint16_t range = packet[7 + i * 2] * 256 + packet[7 + i * 2 + 1];
        uint16_t intensity = packet[7 + i * 2 + 2];
        current_scan_.ranges[i] = range / 100.0;
        current_scan_.intensities[i] = intensity;
    }

    scan_ready_ = true;

    LOG_INFO("[%s][%s][%d] Parsed scan: angle_min=%.2f, angle_max=%.2f, angle_increment=%.2f", __FILE__, __FUNCTION__, __LINE__, current_scan_.angle_min, current_scan_.angle_max, current_scan_.angle_increment);
    LOG_INFO("[%s][%s][%d] Parsed scan: ranges[0]=%.2f, ranges[1]=%.2f, ranges[2]=%.2f", __FILE__, __FUNCTION__, __LINE__, current_scan_.ranges[0], current_scan_.ranges[1], current_scan_.ranges[2]);
    LOG_INFO("[%s][%s][%d] Parsed scan: intensities[0]=%.2f, intensities[1]=%.2f, intensities[2]=%.2f", __FILE__, __FUNCTION__, __LINE__, current_scan_.intensities[0], current_scan_.intensities[1], current_scan_.intensities[2]);
    LOG_INFO("[%s][%s][%d] Parsed scan: ranges size=%zu, intensities size=%zu", __FILE__, __FUNCTION__, __LINE__, current_scan_.ranges.size(), current_scan_.intensities.size());
}

}
