#include <memory>
#include <string>
#include <vector>

#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class ConfigurableNode : public rclcpp::Node {
public:
    ConfigurableNode() : Node("configurable_node") {
        this->declare_parameter("robot_name", "robot");
        this->declare_parameter("max_speed", 0.5);

        on_set_cb_ = this->add_on_set_parameters_callback(
            [this](const std::vector<rclcpp::Parameter> &params) {
                rcl_interfaces::msg::SetParametersResult result;
                result.successful = true;
                for (const auto &param : params) {
                    if (param.get_name() == "max_speed" && param.as_double() < 0.0) {
                        result.successful = false;
                        result.reason = "max_speed must be non_negative";
                    }
                }
                return result;
            }
        );

        timer_ = this->create_wall_timer(
            1s, [this]() {
                auto name = this->get_parameter("robot_name").as_string();
                auto speed = this->get_parameter("max_speed").as_double();
                RCLCPP_INFO(this->get_logger(), "%s running, max_speed = %.2f", name.c_str(), speed);
            }
        );
    }
private:
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr on_set_cb_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ConfigurableNode>());
    rclcpp::shutdown();
    return 0;
}