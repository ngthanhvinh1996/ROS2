#ifndef ROBOT_CAM_HPP_
#define ROBOT_CAM_HPP_

#include "rclcpp/rclcpp.hpp"
#include "hobot_mipi_node.hpp" 
#include "x5/hobot_mipi_cap_iml.hpp"
#include "x5/hobot_mipi_factory.hpp"
#include "hobot_mipi_comm.hpp"

namespace robot_cam
{
    
class RobotCamera {
public:
    Robot_Camera(const rclcpp::Node_Options &node_options);
    ~Robot_Camera();
    static std::share_ptr<RobotCamera> create_camera();
    virtual int init();
    virtual int deInit();
    virtual int start();
    virtual int stop();
    virtual bool getImage();
    virtual bool getCombineImage();
    virtual bool isCapturing();
private:
    virtual void time_callback();
};

} // robot_cam



#endif /* ROBOT_CAM_HPP_ */