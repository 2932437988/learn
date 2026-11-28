# 发送端 steady_clock 纳秒。同一台 Linux 上跨进程可比（都是 CLOCK_MONOTONIC）
int64 stamp_ns
# 发布序号，用来数应用层看到的序号跳变
int32 seq