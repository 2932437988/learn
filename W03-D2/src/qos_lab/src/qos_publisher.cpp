#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "my_interfaces/msg/qos_probe.hpp"
#include "rclcpp/qos.hpp"
#include "rclcpp/qos_event.hpp"
#include "rclcpp/rclcpp.hpp"

using my_interfaces::msg::QosProbe;

namespace {

rclcpp::QoS make_qos(int depth, const std::string & reliability,
                    int deadline_ms, int lifespan_ms,
                    const std::string & liveliness, int lease_ms) {
    rclcpp::QoS qos(rclcpp::KeepLast(static_cast<size_t>(depth < 1 ? 1 : depth)));
    if (reliability == "best_effort") {
        qos.best_effort();
    } else {
        qos.reliable();
    }
    if (deadline_ms > 0) {
        qos.deadline(std::chrono::milliseconds(deadline_ms));
    }
    if (lifespan_ms > 0) {
        qos.deadline(std::chrono::milliseconds(lifespan_ms));
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

}

class QosPublisher : public rclcpp::Node {
public:
    QosPublisher() : Node("qos_publisher") {
        this->declare_parameter("publish_hz", 100.0);
        this->declare_parameter("depth", 10);
        this->declare_parameter("reliability", std::string("reliable"));
        this->declare_parameter("deadline_ms", 0);
        this->declare_parameter("lifespan_ms", 0);
        this->declare_parameter("liveliness", std::string("automatic"));
        this->declare_parameter("lease_ms", 0);
        this->declare_parameter("assert_liveliness", true);

        const double hz = this->get_parameter("publish_hz").as_double();
        const int depth = this->get_parameter("depth").as_int();
        const std::string reliability = this->get_parameter("reliability").as_string();
        const int deadline_ms = this->get_parameter("deadline_ms").as_int();
        const int lifespan_ms = this->get_parameter("lifespan_ms").as_int();
        const std::string liveliness = this->get_parameter("liveliness").as_string();
        const int lease_ms = this->get_parameter("lease_ms").as_int();
        assert_liveliness_ = this->get_parameter("assert_liveliness").as_bool();
        liveliness_manual_ = (liveliness == "manual");

        const auto qos = make_qos(depth, reliability, deadline_ms, lifespan_ms, liveliness, lease_ms);

        rclcpp::PublisherOptions pub_options;
        pub_options.event_callbacks.deadline_callback =
        [this](rclcpp::QOSDeadlineOfferedInfo & info) {
          RCLCPP_WARN(this->get_logger(),
                      "offered deadline missed: total=%d change=%d",
                      info.total_count, info.total_count_change);
        };
        pub_options.event_callbacks.liveliness_callback =
        [this](rclcpp::QOSLivelinessLostInfo & info) {
          RCLCPP_WARN(this->get_logger(),
                      "liveliness lost: total=%d change=%d",
                      info.total_count, info.total_count_change);
        };
        pub_options.event_callbacks.incompatible_qos_callback =
        [this](rclcpp::QOSOfferedIncompatibleQoSInfo & info) {
          RCLCPP_ERROR(this->get_logger(),
                       "offered incompatible QoS: total=%d policy=%d",
                       info.total_count,
                       static_cast<int>(info.last_policy_kind));
        };

        pub_ = this->create_publisher<QosProbe>("/qos_probe", qos, pub_options);

        const double rate = hz > 0.0 ? hz : 100.0;
        const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double>(1.0 / rate));
        timer_ = this->create_wall_timer(period, [this]() {publish_once(); });

        RCLCPP_INFO(this->get_logger(),
                "publisher ready: hz=%.1f depth=%d reliability=%s "
                "deadline=%dms lifespan=%dms liveliness=%s lease=%dms",
                rate, depth, reliability.c_str(), deadline_ms, lifespan_ms,
                liveliness.c_str(), lease_ms);
    }

private:
    void publish_once() {
        QosProbe msg;
        msg.seq = ++seq_;
        msg.stamp_ns = static_cast<int64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
        pub_->publish(msg);
        if (liveliness_manual_ && assert_liveliness_) {
        // assert_liveliness() 带 RCUTILS_WARN_UNUSED，必须消费返回值。
        // 返回 false 说明当前 rmw 没能完成续租；用节流日志避免 100 Hz 刷屏。
            if (!pub_->assert_liveliness()) {
                RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                             "assert_liveliness failed: rmw may not support "
                             "MANUAL_BY_TOPIC");
            }
        }
        if (seq_ % 100 == 0) {
            RCLCPP_INFO(this->get_logger(), "published %d messages", seq_);
        }
    }

    rclcpp::Publisher<QosProbe>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    int32_t seq_ = 0;
    bool liveliness_manual_ = false;
    bool assert_liveliness_ = true;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<QosPublisher>());
    rclcpp::shutdown();
    return 0;
}