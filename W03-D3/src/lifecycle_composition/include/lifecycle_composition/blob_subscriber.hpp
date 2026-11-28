#ifndef LIFECYCLE_COMPOSITION__BLOB_SUBSCRIBER_HPP_
#define LIFECYCLE_COMPOSITION__BLOB_SUBSCRIBER_HPP_

#include <cstdint>
#include <memory>

#include "my_interfaces/msg/blob_probe.hpp"
#include "rclcpp/rclcpp.hpp"

namespace lifecycle_composition
{

class BlobSubscriber : public rclcpp::Node
{
public:
  explicit BlobSubscriber(const rclcpp::NodeOptions & options);

private:
  void on_message(const my_interfaces::msg::BlobProbe::ConstSharedPtr msg);

  rclcpp::Subscription<my_interfaces::msg::BlobProbe>::SharedPtr sub_;
  int64_t received_ = 0;
  int64_t gap_ = 0;
  int32_t last_seq_ = -1;
  double lat_sum_ = 0.0;
  double lat_max_ = 0.0;
  int log_every_ = 100;
};

}  // namespace lifecycle_composition

#endif  // LIFECYCLE_COMPOSITION__BLOB_SUBSCRIBER_HPP_