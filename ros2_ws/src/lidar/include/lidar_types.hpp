#ifndef LIDAR_TYPES_HPP_
#define LIDAR_TYPES_HPP_

#include <string>
#include <vector>
#include <cstdint>

namespace lidar {

#define PACKET_HEADER_BYTE_0 0xA5
#define PACKET_HEADER_BYTE_1 0x5A

enum class ParseState {
    WAIT_HEADER,
    WAIT_LENGTH,
    WAIT_DATA,
    WAIT_CHECKSUM,
};

struct ScanPoint {
    float x = 0.0f;
    float y = 0.0f;
};

struct ScanData {
    float angle = 0.0f;
    float range = 0.0f;
    float intensity = 0.0f;
};

struct LidarConfig {
    std::string port_name;
    int baudrate;
    std::string frame_id;
    std::string scan_topic;
    float min_range;
    float max_range;
    uint8_t valid_point;
};

}

#endif /* LIDAR_TYPES_HPP_ */
