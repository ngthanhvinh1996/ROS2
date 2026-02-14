#include "robot_camera.hpp"

namespace robot_cam
{
class RobotCameraIml : public RobotCamera 
{
public:
    RobotCameraIml();
    ~RobotCameraIml();
    int init(std::shared_ptr<struct mipi_cam::NodePara> para) override;
    int deInit() override;
    int start() override;
    int stop() override;
    bool getImage(builtin_interfaces::msg::Time &stamp,
                             std::string &endcoding,
                             uint32_t &height,
                             uint32_t &width,
                             uint32_t &step,
                             std::vector<uint8_t> &data, std::string channel) override;
    bool getCamCalibration(sensor_msgs::msg::CameraInfo& cam_info,
                           const std::string &file_path) override;
    bool getDualCamCalibration(sensor_msgs::msg::CameraInfo &cam_info_l,
                sensor_msgs::msg::CameraInfo &cam_info_r, const std::string &file_path) override;
    bool isCapturing() override;
private:
    inline void NV12_TO_BGR24(unsigned char *_src, unsigned char *_RGBOut, int width, int height);
    bool lsInit_;
    bool is_capturing_;
    std::shared_ptr<struct mipi_cam::NodePara> nodePara_;
    std::shared_ptr<mipi_cam::HobotMipiCap> robotCap_ptr_;
    mipi_cam::MIPI_CAP_INFO_ST cap_info_;
    bool getDualCamCalibrationIml(sensor_msgs::msg::CameraInfo &cam_info_l, sensor_msgs::msg::CameraInfo &cam_info_r, const std::string &file_path);
    bool getCamCalibrationIml(sensor_msgs::msg::CameraInfo& cam_info,const std::string &file_path);

    typedef struct camera_image_s 
    {
        int width;
        int height;
        int image_size;
        char *image;
        ~camera_image_s(){
            if(image != NULL) {
                free(image);
                image = NULL;
            }
        }
    } camera_image_t;

    camera_image_t *image_nv12_ = nullptr;
    std::mutex image_nv12_mtx_;
};

std::shared_ptr<RobotCamera> RobotCamera::create_camera()
{
    return std::make_shared<RobotCameraIml>();
}

RobotCameraIml::RobotCameraIml()
    : lsInit_(false),
      is_capturing_(false)
{
}

RobotCameraIml::~RobotCameraIml() 
{
    stop();
    deInit();
}

int RobotCameraIml::init(std::shared_ptr<struct mipi_cam::NodePara> para)
{
    if(lsInit_)
    {
        return 0;
    }
    nodePara_ = para;

    auto board_type = mipi_cam::getBoardType();
    robotCap_ptr_ = mipi_cam::createMipiCap(board_type);
    if(nullptr == robotCap_ptr_)
    {
        RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "[%s]->cap %s create capture failture.\r\n", __func__, board_type.c_str());
        return -1;
    }
    cap_info_.config_path = nodePara_->config_path_;
    cap_info_.sensor_type = nodePara_->video_device_name_;
    cap_info_.width = nodePara_->image_width_;
    cap_info_.height = nodePara_->image_height_;
    cap_info_.sub_width = nodePara_->sub_image_width_;
    cap_info_.sub_height = nodePara_->sub_image_height_;
    cap_info_.fps = nodePara_->framerate_;
    cap_info_.channel_ = nodePara_->channel_;
    cap_info_.channel2_ = nodePara_->channel2_;
    cap_info_.device_mode_ = nodePara_->device_mode_;
    cap_info_.dual_combine_ = nodePara_->dual_combine_;
    cap_info_.lpwm_enable_ = nodePara_->lpwm_enable_;
    cap_info_.gdc_bin_file_ = nodePara_->gdc_bin_file_;
    cap_info_.rotation_ = nodePara_->rotation_;
    cap_info_.cal_rotation_ - nodePara_->cal_rotation_;
    cap_info_.gdc_enable_ = nodePara_->gdc_enable_;
    cap_info_.frame_ts_type_ = nodePara_->frame_ts_type_;
    cap_info_.link_type_ = nodePara_->link_type_;
    cap_info_.link_port_ = nodePara_->link_port_;
    cap_info_.cal_alpha_ = nodePara_->cal_alpha_;
    cap_info_.sub_stream_flag_ = nodePara_->sub_stream_flag_;

    if(0 > robotCap_ptr_->initEnv())
    {
        RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "[%s]->init %s's robot host and gpio failture.\r\n", __func__, board_type.c_str());
        return -1;
    }
    
    if("dual" == cap_info_.device_mode_)
    {
        std::vector<sensor_msgs::msg::CameraInfo> cam_info_v;
        cam_info_v.resize(2);
        if(getDualCamCalibrationIml(cam_info_v[0], cam_info_v[1], nodePara_->camera_calibration_file_path_))
        {
            robotCap_ptr_->setCamInfo(cam_info_v);
        }
    }
    else
    {
        std::vector<sensor_msgs::msg::CameraInfo> cam_info_v;
        cam_info_v.resize(1);
        if(getCamCalibrationIml(cam_info_v[0], nodePara_->camera_calibration_file_path_))
        {
            robotCap_ptr_->setCamInfo(cam_info_v);
        }
    }

    if(0 != robotCap_ptr_->init(cap_info_))
    {
        RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "[%s]->cap capture init failture.\r\n", __func__);
        return -5;
    }

    robotCap_ptr_->getCapInfo(cap_info_);
    nodePara_->image_width_ = cap_info_.width;
    nodePara_->image_height_ - cap_info_.height;
    nodePara_->video_device_name_ = cap_info_.sensor_type;
    nodePara_->sub_stream_flag_ = cap_info_.sub_stream_flag_;

    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "[%s]->cap %s init success.\r\n", __func__, cap_info_.sensor_type.c_str());
    lsInit_ = true;
    return 0;
}

int RobotCameraIml::deInit()
{
    int ret = 0;
    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "robot_cam deInit start");
    if(lsInit_)
    {
        lsInit_ = false;
        if(true == is_capturing_)
        {
            stop();
        }
        if(nullptr != image_nv12_)
        {
            free(image_nv12_);
            image_nv12_ = nullptr;
        }

        ret = robotCap_ptr_->deInit();
        robotCap_ptr_ = nullptr;
    }
    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "robot_cam deInit end");
    return ret;
}

int RobotCameraIml::start()
{
    if(!lsInit_ || is_capturing_)
    {
        return -1;
    }

    int ret = 0;
    if(robotCap_ptr_->start())
    {
        RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "[%s]->cap capture start failture.\r\n");
        return -1;
    }

    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "[%s]->w:h=%d:%d.\r\n", __func__, nodePara_->image_width_, nodePara_->image_height_);
    is_capturing_ = true;
    if("bgr8" == nodePara_->out_format_name_)
    {
        image_nv12_ = reinterpret_cast<camera_image_t *>(calloc(1, sizeof(camera_image_t)));
        image_nv12_->width = nodePara_->image_width_;
        image_nv12_->height = nodePara_->image_height_;
        image_nv12_->image_size = nodePara_->image_width_ * nodePara_->image_height_ * 1.5 * 2;
        image_nv12_->image = reinterpret_cast<char *>(calloc(image_nv12_->image_size, sizeof(char *)));
    }

    return ret;
}

int RobotCameraIml::stop()
{
    int ret = 0;
    if(true == is_capturing_)
    {
        ret = robotCap_ptr_->stop();
    }
    is_capturing_ = false;
    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "robot_cam is stoped");
    return ret;
}

bool RobotCameraIml::getImage(builtin_interfaces::msg::Time &stamp,
                                std::string &encoding,
                                uint32_t &height,
                                uint32_t &width,
                                uint32_t &step,
                                std::vector<uint8_t> &data, std::string channel)
{
    if(!is_capturing_)
    {
        RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "[%s][%-%d] Camera isn't capturing", __FILE__, __func__, __LINE__);
        return false;
    }

    if((0 == nodePara_->image_width_) || (0 == nodePara_->image_height_))
    {
        RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "Invalid publish width:%d height:%d! Please check the image_width "
            "and image_height parameters!", nodePara_->image_width_, nodePara_->image_height_);
        return false;
    }

    struct timespec time_start = {0, 0};
    int64_t msStart (0), msEnd (0);
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        msStart = (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    }
    
    uint64_t timestamp;
    int data_size;

    if("combine" == channel)
    {
        data_size = nodePara_->image_width_ * nodePara_->image_height_ * 1.5 * 2;
    }
    else
    {
        data_size = nodePara_->image_width_ * nodePara_->image_height_ * 1.5;
    }

    if(("bgr8" == nodePara_->out_format_name_) && image_nv12_)
    {
        std::lock_guard<std::mutex> lck(image_nv12_mtx_);
        if(robotCap_ptr_->getFrame(channel,
                                   reinterpret_cast<int *>(&width),
                                   reinterpret_cast<int *>(&height),
                                   reinterpret_cast<void *>(image_nv12_->image),
                                   image_nv12_->image_size,
                                   reinterpret_cast<unsigned int *>(&data_size),
                                   timestamp))
        {
            return false;
        }
        data_size = width * height * 3;
        data.resize(data_size);

        uint64_t msStart_bgr (0), msEnd_bgr (0);
        {
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            msStart_bgr = (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        }
        NV12_TO_BGR24((unsigned char *)image_nv12_->image, (unsigned char *)&data[0], width, height);
        encoding = "bgr8";
        step = width * 3;
        {
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            msEnd_bgr = (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        }
        RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "NV12_TO_BGR24 laps ms=%ld", (msEnd_bgr - msStart_bgr));
    }
    else if("gray" == nodePara_->out_format_name_)
    {
        data_size = nodePara_->image_width_ * nodePara_->image_height_;
        data.resize(data_size);
        if(robotCap_ptr_->getFrame(channel,
                                   reinterpret_cast<int *>(&width),
                                   reinterpret_cast<int *>(&height),
                                   reinterpret_cast<void *>(&data[0]),
                                   data_size,
                                   reinterpret_cast<unsigned int *>(&data_size),
                                   timestamp))
        {
            return false;
        }
        encoding = "mono8";
        step = width;
    }
    else
    {
        data.resize(data_size);
        if(robotCap_ptr_->getFrame(channel,
                                   reinterpret_cast<int *>(&width),
                                   reinterpret_cast<int *>(&height),
                                   reinterpret_cast<void *>(&data[0]),
                                   data_size,
                                   reinterpret_cast<unsigned int *>(&data_size),
                                   timestamp))
        {
            return false;
        }
        encoding = "nv12",
        step = width;
    }
    stamp.sec = timestamp / 1e9;
    stamp.nanosec = timestamp - stamp.sec * 1e9;

    uint64_t timestamp_sys;
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        timestamp_sys = (tv.tv_sec * 1000 + tv.tv_usec/1000);
    }

    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "publish lap ms=%lu", (timestamp_sys - timestamp/10000000)); 

    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        msEnd = (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    }

    // RCLCPP_INFO_STREAM(rclcpp::get_logger("robot_cam"),
    //                     "getImage channel=" << channel.data()
    //                     << ", enc=" << encoding.data()
    //                     << ", width=" << width
    //                     << ", height=" << height
    //                     << ", step=" << step
    //                     << ", sz=" << data_size
    //                     << std::fixed
    //                     << ", ts=" << stamp.sec + stamp.nanosec * 1e9
    //                     << ", laps ms=" << msEnd - msStart);
    return true;
}

bool RobotCameraIml::getCamCalibration(sensor_msgs::msg::CameraInfo& cam_info,
                           const std::string &file_path)
{
    if(!robotCap_ptr_)
    {
        RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "[%s]-> robotCap_ptr_ is NULL", __func__);
        return false;
    }

    auto cal_v_ptr = robotCap_ptr_->getCalCamInfo();
    if((nullptr != cal_v_ptr) && (0 < cal_v_ptr->size()))
    {
        RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "[%s]->get calibration cam info", __func__);
        const sensor_msgs::msg::CameraInfo &cameraInfo = cal_v_ptr->at(0);
        memcpy(&cam_info, &cameraInfo, sizeof(sensor_msgs::msg::CameraInfo));
        return true;
    }
    else
    {
        return getCamCalibrationIml(cam_info, file_path);
    }
}

bool RobotCameraIml::isCapturing()
{
    return is_capturing_;
}

bool RobotCameraIml::getDualCamCalibration(sensor_msgs::msg::CameraInfo &cam_info_l,
                sensor_msgs::msg::CameraInfo &cam_info_r, const std::string &file_path)
{
    if(!robotCap_ptr_)
    {
        return false;
    }

    auto cal_v_ptr = robotCap_ptr_->getCalCamInfo();
    if((nullptr != cal_v_ptr) && (2 == cal_v_ptr->size()))
    {
        RCLCPP_INFO(rclcpp::get_logger("robot_cap"), "get calibration camera info");
        const sensor_msgs::msg::CameraInfo&cal_l = cal_v_ptr->at(0);
        memcpy(&cam_info_l, &cal_l, sizeof(sensor_msgs::msg::CameraInfo));
        const sensor_msgs::msg::CameraInfo& cal_r = cal_v_ptr->at(1);
        memcpy(&cam_info_r, &cal_r, sizeof(sensor_msgs::msg::CameraInfo));
        return true;
    } 
    else
    {
        return getDualCamCalibrationIml(cam_info_l, cam_info_r, file_path);
    }
}

bool RobotCameraIml::getDualCamCalibrationIml(sensor_msgs::msg::CameraInfo &cam_info_l, 
                                                sensor_msgs::msg::CameraInfo &cam_info_r, 
                                                const std::string &file_path)
{
    RCLCPP_INFO(rclcpp::get_logger("robot_cam"), "cal_file:%s", file_path.c_str());

    try {
        if(0 == file_path.length() || "default" == file_path)
        {
            RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "Wrong file path:%s", file_path.c_str());
            return false;
        }

        cv::FileStorage fs(file_path.c_str(), cv::FileStorage::READ);
        if(!fs.isOpened())
        {
            RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), 
                "Camera calibration file: %s is not exist"
                "\nIf you need calibration msg, please make sure the calibration file path is correct and the calibration file exists",
                file_path.c_str());
            return false;
        }

        cv::Mat l_k, l_d, r_k, r_d, R, T;

        int width = fs["image_width"];
        int height = fs["image_height"];
        fs["left_camera_matrix"] >> l_k;
        fs["left_distortion_coefficients"] >> l_d;
        fs["right_camera_matix"] >> r_k;
        fs["right_distortion_coefficients"] >> r_d;
        fs["R"] >> R;
        fs["T"] >> T;
        fs.release();

        if(CV_64F != l_k.type())
        {
            l_k.convertTo(l_k, CV_64F);
        }
        if(CV_64F != l_d.type())
        {
            l_d.convertTo(l_d, CV_64F);
        }
        if(CV_64F != r_k.type())
        {
            r_k.convertTo(r_k, CV_64F);
        }
        if(CV_64F != r_d.type())
        {
            r_d.convertTo(r_d, CV_64F);
        }
        if(CV_64F != R.type())
        {
            R.convertTo(R, CV_64F);
        }
        if(CV_64F != T.type())
        {
            T.convertTo(T, CV_64F);
        }

        cam_info_r.width = cam_info_l.width = width;
        cam_info_r.height = cam_info_l.height = height;

        cam_info_l.d.resize(l_d.total());
        std::copy(l_d.ptr<double>(0), l_d.ptr<double>(0) + l_d.total(), cam_info_l.d.begin());
        std::copy(l_k.ptr<double>(0), l_k.ptr<double>(0) + l_k.total(), cam_info_l.k.begin());

        cam_info_r.d.resize(r_d.total());
        std::copy(r_d.ptr<double>(0), r_d.ptr<double>(0) + r_d.total(), cam_info_r.d.begin());
        std::copy(r_k.ptr<double>(0), r_k.ptr<double>(0) + r_k.total(), cam_info_r.k.begin());

        cv::Mat l_r_eye = cv::Mat::eye(3, 3, CV_64F);
        std::copy(l_r_eye.ptr<double>(0), l_r_eye.ptr<double>(0) + l_r_eye.total(), cam_info_l.r.begin());

        cv::Mat l_p_eye = cv::Mat::eye(3, 4, CV_64F);
        cv::Mat l_p = l_k * l_p_eye;
        std::copy(l_p.ptr<double>(0), l_p.ptr<double>(0) + l_p.total(), cam_info_l.p.begin());
        
        cv::Mat RT = cv::Mat::zeros(3, 4, CV_64F);
        R.copyTo(RT(cv::Rect(0, 0, 3, 3)));
        T.reshape(1).copyTo(RT.col(3));
        cv::Mat P = r_k * RT;
        std::copy(R.ptr<double>(0), R.ptr<double>(0) + R.total(), cam_info_r.r.begin());
        std::copy(P.ptr<double>(0), P.ptr<double>(0) + P.total(), cam_info_r.p.begin());
        fs.release();
        return true;
    } catch (cv::Exception &ex) {
        RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "cv_bridge exception: %s", ex.what());
        return false;
    }
}

bool RobotCameraIml::getCamCalibrationIml(sensor_msgs::msg::CameraInfo& cam_info,
                                            const std::string &file_path)
{
    try {
        std::string cal_file;
        if((0 == file_path.length()) || ("default" == file_path))
        {
            mipi_cam::MIPI_CAP_INFO_ST cap_info;

            robotCap_ptr_->getCapInfo(cap_info);
            std::string sensor_name = cap_info.sensor_type;
            std::transform(sensor_name.begin(), sensor_name.end(), sensor_name.begin(), [](unsigned char c ){
                return std::toupper(c);
            });
            cal_file = cap_info.config_path + "/" + sensor_name + "_calibration.yaml:";   
        }
        else {
            cal_file = file_path;
        }
        std::string camera_name;
        std::ifstream fin(cal_file.c_str());
        if(!fin) {
            RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), 
                        "Camera calibration file: %s is not exist"
                        "\nIf you need calibration msg, please make sure the calibration file path is correct and the the calibration file exists",
                        cal_file.c_str());
            return false;
        }

        YAML::Node calibration_doc = YAML::Load(fin);
        if(calibration_doc["camera_name"])
        {
            camera_name = calibration_doc["camera_name"].as<std::string>();
        }
        else
        {
            camera_name = "unknown";
        }

        cam_info.width = calibration_doc["image_width"].as<int>();
        cam_info.height = calibration_doc["image_height"].as<int>();

        const YAML::Node &camera_matrix = calibration_doc["camera_matrix"];
        const YAML::Node &camera_matrix_data = camera_matrix["data"];
        for(int i = 0; i < 9; i++) {
            cam_info.k[i] = camera_matrix_data[i].as<double>();
        }

        const YAML::Node &rectification_matrix = 
                calibration_doc["rectification_matrix"];
        const YAML::Node &rectification_matrix_data = rectification_matrix["data"];
        for (int i = 0; i < 9; i++) {
            cam_info.r[i] = rectification_matrix_data[i].as<double>();
        }
        const YAML::Node &projection_matrix = calibration_doc["projection_matrix"];
        const YAML::Node &projection_matrix_data = projection_matrix["data"];
        for (int i = 0; i < 12; i++) {
            cam_info.p[i] = projection_matrix_data[i].as<double>();
        }

        if (calibration_doc["distortion_model"]) {
            cam_info.distortion_model =
                calibration_doc["distortion_model"].as<std::string>();
        } else {
            cam_info.distortion_model = sensor_msgs::distortion_models::PLUMB_BOB;
            RCLCPP_INFO(rclcpp::get_logger("mipi_cam"),
                        "Camera calibration file did not specify distortion model, "
                        "assuming plumb bob");
        }
        const YAML::Node &distortion_coefficients =
            calibration_doc["distortion_coefficients"];
        int d_rows, d_cols;
        d_rows = distortion_coefficients["rows"].as<int>();
        d_cols = distortion_coefficients["cols"].as<int>();
        const YAML::Node &distortion_coefficients_data =
            distortion_coefficients["data"];
        cam_info.d.resize(d_rows * d_cols);
        for (int i = 0; i < d_rows * d_cols; ++i) {
            cam_info.d[i] = distortion_coefficients_data[i].as<double>();
        }
        RCLCPP_INFO(rclcpp::get_logger("mipi_cam"),
        "[getCamCalibration]->parse calibration file successfully");
        return true;
    } catch (YAML::Exception &e) {
        RCLCPP_ERROR(rclcpp::get_logger("robot_cam"), "Unable to parse camera calibration file normally: %s",
                        e.what());
        return false;
    }
}

inline void RobotCameraIml::NV12_TO_BGR24(unsigned char *_src, unsigned char *_RGBOut, int width, int height)
{
    cv::Mat src(height * 3 / 2, width, CV_8UC1, (void*)_src);
    cv::Mat bgr_mat;
    cv::cvtColor(src, bgr_mat, cv::COLOR_YUV2BGR_NV12);
    memcpy(_RGBOut, bgr_mat.ptr<uint8_t>(), height * width * 3);
    return;
}

} // robot_cam