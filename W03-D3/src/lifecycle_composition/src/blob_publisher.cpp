#include "lifecycle_composition/blob_publisher.hpp"

#include <utility>

#include "rclcpp_components/register_node_macro.hpp"

namespace lifecycle_composition
{

BlobPublisher::BlobPublisher(const rclcpp::NodeOptions & options)
: rclcpp::Node("blob_publisher", options)
{
  this->declare_parameter("publish_hz", 100.0);
  this->declare_parameter("payload_size", 65536);

  const double hz = this->get_parameter("publish_hz").as_double();
  payload_size_ = static_cast<size_t>(this->get_parameter("payload_size").as_int());

  pub_ = this->create_publisher<my_interfaces::msg::BlobProbe>(
    "/blob_topic", rclcpp::QoS(10));

  const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::duration<double>(1.0 / (hz > 0.0 ? hz : 1.0)));
  timer_ = this->create_wall_timer(period, [this]() {this->publish_once();});

  RCLCPP_INFO(get_logger(),
    "BlobPublisher ready: hz=%.1f payload=%zu intra=%d",
    hz, payload_size_,
    static_cast<int>(this->get_node_options().use_intra_process_comms()));
}

void BlobPublisher::publish_once()
{
  // intra-process 零拷贝的前提：用 unique_ptr 发布，把所有权直接交给进程内订阅者。
  auto msg = std::make_unique<my_interfaces::msg::BlobProbe>();
  msg->stamp_ns = static_cast<int64_t>(
    std::chrono::steady_clock::now().time_since_epoch().count());
  msg->seq = seq_++;
  msg->payload.assign(payload_size_, 0x5A);
  pub_->publish(std::move(msg));
}

}  // namespace lifecycle_composition

RCLCPP_COMPONENTS_REGISTER_NODE(lifecycle_composition::BlobPublisher)