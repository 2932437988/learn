#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

#include "my_interfaces/msg/qos_probe.hpp"
#include "rclcpp/qos.hpp"
#include "rclcpp/qos_event.hpp"
#include "rclcpp/rclcpp.hpp"

using my_interfaces::msg::QosProbe;

namespace {

rclcpp::QoS make_qos(int depth, const std::string & reliability,
                     int deadline_ms, const std::string & liveliness,
                     int lease_ms) {
  rclcpp::QoS qos(rclcpp::KeepLast(static_cast<size_t>(depth < 1 ? 1 : depth)));
  if (reliability == "best_effort") {
    qos.best_effort();
  } else {
    qos.reliable();
  }
  if (deadline_ms > 0) {
    qos.deadline(std::chrono::milliseconds(deadline_ms));
  }
  if (liveliness == "manual") {
    qos.liveliness(rclcpp::LivelinessPolicy::ManualByTopic);
  } else {
    qos.liveliness(rclcpp::LivelinessPolicy::Automatic);
  }
  if (lease_ms > 0) {
    qos.liveliness_lease_duration(std::chrono::milliseconds(lease_ms));
  }
  return qos;
}

}  // namespace

class QosSubscriber : public rclcpp::Node {
public:
  QosSubscriber() : Node("qos_subscriber") {
    this->declare_parameter("depth", 10);
    this->declare_parameter("reliability", std::string("reliable"));
    this->declare_parameter("deadline_ms", 0);
    this->declare_parameter("liveliness", std::string("automatic"));
    this->declare_parameter("lease_ms", 0);
    this->declare_parameter("slow_ms", 0);
    this->declare_parameter("log_every", 100);

    const int depth = this->get_parameter("depth").as_int();
    const std::string reliability =
        this->get_parameter("reliability").as_string();
    const int deadline_ms = this->get_parameter("deadline_ms").as_int();
    const std::string liveliness =
        this->get_parameter("liveliness").as_string();
    const int lease_ms = this->get_parameter("lease_ms").as_int();
    slow_ms_ = this->get_parameter("slow_ms").as_int();
    log_every_ = this->get_parameter("log_every").as_int();
    if (log_every_ < 1) {
      log_every_ = 1;
    }

    const auto qos =
        make_qos(depth, reliability, deadline_ms, liveliness, lease_ms);

    rclcpp::SubscriptionOptions sub_options;
    sub_options.event_callbacks.deadline_callback =
        [this](rclcpp::QOSDeadlineRequestedInfo & info) {
          RCLCPP_WARN(this->get_logger(),
                      "requested deadline missed: total=%d change=%d",
                      info.total_count, info.total_count_change);
        };
    sub_options.event_callbacks.liveliness_callback =
        [this](rclcpp::QOSLivelinessChangedInfo & info) {
          RCLCPP_WARN(this->get_logger(),
                      "liveliness changed: alive=%d not_alive=%d "
                      "change=%d/%d",
                      info.alive_count, info.not_alive_count,
                      info.alive_count_change, info.not_alive_count_change);
        };
    sub_options.event_callbacks.incompatible_qos_callback =
        [this](rclcpp::QOSRequestedIncompatibleQoSInfo & info) {
          RCLCPP_ERROR(this->get_logger(),
                       "requested incompatible QoS: total=%d policy=%d",
                       info.total_count,
                       static_cast<int>(info.last_policy_kind));
        };

    sub_ = this->create_subscription<QosProbe>(
        "/qos_probe", qos,
        [this](const QosProbe & msg) { on_message(msg); }, sub_options);

    RCLCPP_INFO(this->get_logger(),
                "subscriber ready: depth=%d reliability=%s "
                "deadline=%dms liveliness=%s lease=%dms slow=%dms "
                "log_every=%d",
                depth, reliability.c_str(), deadline_ms, liveliness.c_str(),
                lease_ms, slow_ms_, log_every_);
  }

private:
  void on_message(const QosProbe & msg) {
    const int64_t now_ns = static_cast<int64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    const double lat_ms = static_cast<double>(now_ns - msg.stamp_ns) / 1e6;

    if (last_seq_ >= 0 && msg.seq > last_seq_ + 1) {
      gap_ += static_cast<int64_t>(msg.seq - last_seq_ - 1);
    }
    last_seq_ = msg.seq;
    ++received_;
    lat_sum_ += lat_ms;
    if (lat_ms > lat_max_) {
      lat_max_ = lat_ms;
    }

    // 故意拖慢回调，模拟处理耗时的业务节点；D4 再讲它为什么会拖住 executor
    if (slow_ms_ > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(slow_ms_));
    }

    if (received_ % log_every_ == 0) {
      RCLCPP_INFO(this->get_logger(),
                  "recv=%ld gap=%ld avg_lat=%.2fms max_lat=%.2fms",
                  static_cast<long>(received_), static_cast<long>(gap_),
                  lat_sum_ / static_cast<double>(received_), lat_max_);
    }
  }

  rclcpp::Subscription<QosProbe>::SharedPtr sub_;
  int32_t last_seq_ = -1;
  int64_t received_ = 0;
  int64_t gap_ = 0;
  double lat_sum_ = 0.0;
  double lat_max_ = 0.0;
  int slow_ms_ = 0;
  int log_every_ = 100;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<QosSubscriber>());
  rclcpp::shutdown();
  return 0;
}