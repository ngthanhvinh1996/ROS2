#ifndef LIDAR_DRIVER_HPP_
#define LIDAR_DRIVER_HPP_

#include "lidar_types.hpp"
#include "lidar_log.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>


namespace lidar {

speed_t mapping_baudrate(int baudrate);

class LidarDriver {
public:
    LidarDriver(const std::string& port, int baud);
    ~LidarDriver();
    bool open();
    void close();
    bool is_open() const;
    ssize_t read_bytes(unsigned char *buffer, size_t len);
    ssize_t write_bytes(const unsigned char *buffer, size_t len);

private:
    int fd_;
    bool is_connected_;
    std::string port_name_;
    int baudrate_;
};

} /* namespace lidar*/

#endif /* LIDAR_DRIVER_HPP_ */
