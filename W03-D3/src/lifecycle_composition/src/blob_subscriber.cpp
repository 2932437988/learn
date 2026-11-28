#include "lifecycle_composition/blob_subscriber.hpp"

#include <chrono>

#include "rclcpp_components/register_node_macro.hpp"

namespace lifecycle_composition
{

BlobSubscriber::BlobSubscriber(const rclcpp::NodeOptions & options)
: rclcpp::Node("blob_subscriber", options)
{
  this->declare_parameter("log_every", 100);
  log_every_ = this->get_parameter("log_every").as_int();
  if (log_every_ < 1) {
    log_every_ = 1;
  }

  sub_ = this->create_subscription<my_interfaces::msg::BlobProbe>(
    "/blob_topic", rclcpp::QoS(10),
    [this](const my_interfaces::msg::BlobProbe::ConstSharedPtr msg) {
      this->on_message(msg);
    });

  RCLCPP_INFO(get_logger(),
    "BlobSubscriber ready: log_every=%d intra=%d",
    log_every_,
    static_cast<int>(this->get_node_options().use_intra_process_comms()));
}

void BlobSubscriber::on_message(const my_interfaces::msg::BlobProbe::ConstSharedPtr msg)
{
  const int64_t now_ns = static_cast<int64_t>(
    std::chrono::steady_clock::now().time_since_epoch().count());
  const double lat_ms = static_cast<double>(now_ns - msg->stamp_ns) / 1e6;

  ++received_;
  if (last_seq_ >= 0 && msg->seq != last_seq_ + 1) {
    ++gap_;
  }
  last_seq_ = msg->seq;
  lat_sum_ += lat_ms;
  if (lat_ms > lat_max_) {
    lat_max_ = lat_ms;
  }

  if (received_ % log_every_ == 0) {
    RCLCPP_INFO(get_logger(),
      "recv=%ld gap=%ld avg_lat=%.3fms max_lat=%.3fms bytes=%zu",
      static_cast<long>(received_), static_cast<long>(gap_),
      lat_sum_ / static_cast<double>(received_), lat_max_, msg->payload.size());
  }
}

}  // namespace lifecycle_composition

RCLCPP_COMPONENTS_REGISTER_NODE(lifecycle_composition::BlobSubscriber)