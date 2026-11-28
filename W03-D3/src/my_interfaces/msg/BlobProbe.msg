# 发送端 steady_clock 纳秒。同一台 Linux 上跨进程可比（都是 CLOCK_MONOTONIC）
int64 stamp_ns
# 发布序号，用来观察序号跳变
int32 seq
# 变长载荷，用于 intra-process 零拷贝对照
uint8[] payload