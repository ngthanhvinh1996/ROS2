#ifndef LIDAR_PARSER_HPP_
#define LIDAR_PARSER_HPP_

#include "lidar_types.hpp"
#include "lidar_log.hpp"

#define LIDAR_HEADER_LENGTH     5U
#define LIDAR_DEGREE_BIT_START  5U
#define LIDAR_DEGREE_BIT_END    105U

namespace lidar {

class LidarParser {
public:
    LidarParser();
    ~LidarParser();
    void feed_bytes(const std::vector<uint8_t> &data, size_t len);
    ScanData get_scan();
    bool has_complete_scan() const;
    void reset();

private:
    bool verify_checksum(const std::vector<uint8_t> &packet, size_t len);
    void parse_packet(const std::vector<uint8_t> &packet, size_t len);

    std::vector<uint8_t> buffer_;
    ParseState state_;
    ScanData current_scan_;
    bool scan_ready_;
};

}

#endif /* LIDAR_PARSER_HPP_ */