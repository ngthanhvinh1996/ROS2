#ifndef LIDAR_TYPES_HPP_
#define LIDAR_TYPES_HPP_

#include <string>
#include <vector>
#include <cstdint>

namespace lidar {

enum class ParseState {
    WAIT_HEADER,
    WAIT_LENGTH,
    WAIT_DATA,
    WAIT_CHECKSUM,
};

struct ScanData {
    float angle_min = 0.0f;
    float angle_max = 0.0f;
    float angle_increment = 0.0f;
    float range_min = 0.15f;
    float range_max = 8.0f;
    std::vector<float> ranges;
    std::vector<float> intensities;
};

struct LidarConfig {
    std::string port_name;
    int baudrate;
    std::string frame_id;
    std::string scan_topic;
};

}

#endif /* LIDAR_TYPES_HPP_ */
