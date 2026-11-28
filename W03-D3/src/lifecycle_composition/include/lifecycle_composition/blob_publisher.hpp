#ifndef LIFECYCLE_COMPOSITION__BLOB_PUBLISHER_HPP_
#define LIFECYCLE_COMPOSITION__BLOB_PUBLISHER_HPP_

#include <chrono>
#include <cstddef>
#include <memory>

#include "my_interfaces/msg/blob_probe.hpp"
#include "rclcpp/rclcpp.hpp"

namespace lifecycle_composition
{

class BlobPublisher : public rclcpp::Node
{
public:
  explicit BlobPublisher(const rclcpp::NodeOptions & options);

private:
  void publish_once();

  rclcpp::Publisher<my_interfaces::msg::BlobProbe>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::size_t payload_size_ = 65536;
  int32_t seq_ = 0;
};

}  // namespace lifecycle_composition

#endif  // LIFECYCLE_COMPOSITION__BLOB_PUBLISHER_HPP_