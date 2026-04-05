#include "lidar_driver.hpp"
#include <cerrno>
#include <cstring>

namespace lidar {

speed_t mapping_baudrate(int baudrate)
{
    switch(baudrate)
    {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
    }

    return B230400;
}

LidarDriver::LidarDriver(const std::string& port, int baud)
    : fd_(-1) 
    , is_connected_(false)
    , port_name_(port)
    , baudrate_(baud)
{

}

LidarDriver::~LidarDriver()
{
    close();
}

bool LidarDriver::open()
{
    if(is_connected_)
    {
        LOG_WARN("[%s][%s][%d] Lidar is already connected", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }

    fd_ = ::open(port_name_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if(-1 == fd_)
    {
        LOG_ERROR("[%s][%s][%d] Failed to open port %s", __FILE__, __FUNCTION__, __LINE__, port_name_.c_str());
        return false;
    }

    if(0 > fcntl(fd_, F_SETFL, 0))
    {
        ::close(fd_);
        LOG_ERROR("[%s][%s][%d] Failed to set flag port %s", __FILE__, __FUNCTION__, __LINE__, port_name_.c_str());
        return false;
    }

    struct termios options;

    if(0 > tcgetattr(fd_, &options))
    {
        ::close(fd_);
        LOG_ERROR("[%s][%s][%d] Failed to get option port %s", __FILE__, __FUNCTION__, __LINE__, port_name_.c_str());
        return false;
    }

    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CRTSCTS;
    options.c_cflag |= CREAD | CLOCAL;
    
    options.c_iflag &= ~(IXON | IXOFF | IXANY | INLCR | IGNCR | ICRNL | ISTRIP | INPCK);

    options.c_oflag &= ~OPOST;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;
    cfsetispeed(&options, mapping_baudrate(baudrate_));
    cfsetospeed(&options, mapping_baudrate(baudrate_));
    tcsetattr(fd_, TCSANOW, &options);
    is_connected_ = true;

    LOG_INFO("[%s][%s][%d] Lidar connected on %s at baudrate %d", __FILE__, __FUNCTION__, __LINE__, port_name_.c_str(), baudrate_);
    return true;
}

void LidarDriver::close()
{
    if(!is_connected_)
    {
        LOG_ERROR("[%s][%s][%d] Failed to close lidar, not connected", __FILE__, __FUNCTION__, __LINE__);
        return;
    }

    ::close(fd_);
    is_connected_ = false;
    fd_ = -1;
    LOG_INFO("[%s][%s][%d] Lidar disconnected", __FILE__, __FUNCTION__, __LINE__);
}

bool LidarDriver::is_open() const
{
    return is_connected_;
}

ssize_t LidarDriver::read_bytes(std::vector<uint8_t>& buffer, size_t offset, size_t len)
{
    if(!is_connected_)
    {
        LOG_ERROR("[%s][%s][%d] Failed to read data from lidar, not connected", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    ssize_t bytes_read = ::read(fd_, buffer.data() + offset, len);
    if(0 > bytes_read)
    {
        LOG_ERROR("[%s][%s][%d] Failed to read data from lidar: %s (errno=%d)", __FILE__, __FUNCTION__, __LINE__, strerror(errno), errno);
        return -1;
    }

    // LOG_INFO("[%s][%s][%d] Read %zd bytes from lidar", __FILE__, __FUNCTION__, __LINE__, bytes_read);
    return bytes_read;
}

ssize_t LidarDriver::write_bytes(const std::vector<uint8_t>& buffer, size_t len)
{
    if(!is_connected_)
    {
        LOG_ERROR("[%s][%s][%d] Failed to write data to lidar, not connected", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    ssize_t bytes_written = ::write(fd_, buffer.data(), len);
    if(0 > bytes_written)
    {
        LOG_ERROR("[%s][%s][%d] Failed to write data to lidar", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    // LOG_INFO("[%s][%s][%d] Write %zd bytes data to lidar", __FILE__, __FUNCTION__, __LINE__, bytes_written);
    return bytes_written;
}

}
