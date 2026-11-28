#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

#include "my_interfaces/msg/blob_probe.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"

using namespace std::chrono_literals;
using CallbackReturn =
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class LifecycleTalker : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit LifecycleTalker(const rclcpp::NodeOptions & options)
  : rclcpp_lifecycle::LifecycleNode("lifecycle_talker", options)
  {
    this->declare_parameter("publish_hz", 20.0);
    this->declare_parameter("payload_size", 4096);
  }

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    const double hz = this->get_parameter("publish_hz").as_double();
    payload_size_ = static_cast<std::size_t>(this->get_parameter("payload_size").as_int());

    pub_ = this->create_publisher<my_interfaces::msg::BlobProbe>(
      "/lifecycle_blob", rclcpp::QoS(10));

    const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::duration<double>(1.0 / (hz > 0.0 ? hz : 1.0)));
    timer_ = this->create_wall_timer(period, [this]() {this->publish_once();});
    timer_->cancel();

    RCLCPP_INFO(get_logger(), "configured: hz=%.1f payload=%zu", hz, payload_size_);
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    pub_->on_activate();
    timer_->reset();
    RCLCPP_INFO(get_logger(), "activated: publishing");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    timer_->cancel();
    pub_->on_deactivate();
    RCLCPP_INFO(get_logger(), "deactivated: timer stopped");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override
  {
    timer_.reset();
    pub_.reset();
    RCLCPP_INFO(get_logger(), "cleaned up: resources released");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override
  {
    timer_.reset();
    pub_.reset();
    RCLCPP_INFO(get_logger(), "shut down");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_error(const rclcpp_lifecycle::State &) override
  {
    if (timer_) {
      timer_->cancel();
    }
    RCLCPP_WARN(get_logger(), "error state: timer cancelled");
    return CallbackReturn::SUCCESS;
  }

private:
  void publish_once()
  {
    auto msg = std::make_unique<my_interfaces::msg::BlobProbe>();
    msg->stamp_ns = static_cast<int64_t>(
      std::chrono::steady_clock::now().time_since_epoch().count());
    msg->seq = seq_++;
    msg->payload.assign(payload_size_, 0x5A);
    pub_->publish(std::move(msg));
  }

  rclcpp_lifecycle::LifecyclePublisher<my_interfaces::msg::BlobProbe>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::size_t payload_size_ = 4096;
  int32_t seq_ = 0;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<LifecycleTalker>(rclcpp::NodeOptions());
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}