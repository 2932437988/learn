#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "my_interfaces/msg/robot_status.hpp"

using namespace std::chrono_literals;

class StatusPublisher : public rclcpp::Node
{
public:
    StatusPublisher()
    : Node("status_publisher"), count_(0)
    {
        publisher_ = this->create_publisher<my_interfaces::msg::RobotStatus>("robot_status", 10);
        timer_ = this->create_wall_timer(
            500ms, std::bind(&StatusPublisher::timer_callback, this)
        );
    }

private:
    void timer_callback()
    {
        auto msg = my_interfaces::msg::RobotStatus();
        msg.robot_name = "mobile_base";
        msg.battery_level = 0.85f;
        msg.linear_velocity = 0.1f * static_cast<float>(count_);
        msg.angular_velocity = 0.0f;
        msg.estop = false;
        RCLCPP_INFO(this->get_logger(), "Publishing status #%zu", count_);
        publisher_->publish(msg);
        ++count_;
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<my_interfaces::msg::RobotStatus>::SharedPtr publisher_;
    std::size_t count_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StatusPublisher>());
    rclcpp::shutdown();
    return 0;
}