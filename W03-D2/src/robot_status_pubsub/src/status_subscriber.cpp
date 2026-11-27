#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "my_interfaces/msg/robot_status.hpp"

class StatusSubscriber : public rclcpp::Node
{
public:
  StatusSubscriber()
  : Node("status_subscriber")
  {
    subscription_ = this->create_subscription<my_interfaces::msg::RobotStatus>(
      "robot_status", 10,
      [this](const my_interfaces::msg::RobotStatus::SharedPtr msg) {
        RCLCPP_INFO(
          this->get_logger(), "Got status from '%s': battery=%.2f, v=%.2f m/s",
          msg->robot_name.c_str(), msg->battery_level, msg->linear_velocity);
      });
  }

private:
  rclcpp::Subscription<my_interfaces::msg::RobotStatus>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StatusSubscriber>());
  rclcpp::shutdown();
  return 0;
}