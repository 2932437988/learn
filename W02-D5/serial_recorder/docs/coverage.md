# 作品集 #0 · 测试覆盖清单

## 1. 运行环境
- Ubuntu 22.04（VMware 客户机）/ GitHub ubuntu-22.04 runner
- g++ 11 / C++17 / GTest 1.11 / spdlog 1.11 / nlohmann-json 3.10
- 命令：ctest --test-dir build --output-on-failure

## 2. 设计契约 → 用例对照

| 契约 / 行为 | 用例 | 判据 | 实测结果 |
| --- | --- | --- | --- |
| 空：消费者阻塞 | RingBuffer.EmptyPopBlocksUntilClose | 50ms 未返回，close 后 nullopt | PASS |
| 满：背压不丢帧 | RingBuffer.FullPushBlocksUntilClose | 50ms 未返回，close 后 false | PASS |
| 断连：唤醒 + 排空 | RingBuffer.CloseDrainsRemaining | 3 帧按序取出，第 4 次 nullopt | PASS |
| 覆盖：不覆盖旧帧 | RingBuffer.FifoOrderPreserved | FIFO 顺序 10/20/30 | PASS |
| 并发正确性 | RingBuffer.SpscNoLossNoDup | count=10000，sum=49995000 | PASS |
| 帧往返 | Frame.RoundTrip | payload 相等，bad=0 | PASS |
| 粘包 / 半包 / 坏帧 | Frame.TwoFramesInOneRead / SplitAcrossReads / BadChecksumCounted | 2 帧 / 解出 / bad=1 | PASS |
| 倍速 1x | PlaybackOffset.SpeedOneKeepsInterval | 100000 → 100000 | PASS |
| 倍速 2x | PlaybackOffset.SpeedTwoHalvesInterval | 100000 → 50000 | PASS |
| 慢放 0.5x | PlaybackOffset.SpeedHalfDoublesInterval | 100000 → 200000 | PASS |
| 基准对齐 | PlaybackOffset.BaseAlignedFirstFrameIsZero | 首帧偏移 0 | PASS |
| JSONL → 字节流 | Player.JsonlRoundTripToBytes | 输出逐字节相等 | PASS |
| 空输入 | Player.EmptyInputReturnsZero | 返回 0 | PASS |

## 3. 汇总
- ctest：15/15 passed
- CI：serial-recorder-ci，4 步全绿，徽章 passing
- valgrind：主程序 / test_ring_buffer / test_player 均 definitely lost=0、ERROR SUMMARY=0、退出码 0
- 已知非问题：spdlog 全局 registry 的 still reachable（不计 error）

## 4. 未覆盖 / 后续
- 真实串口 termios 编程（VM 无硬件，用管道替代）
- 覆盖率统计（gcov/lcov，非本周任务）
- 100 Hz 压测与 P99（W05 之后）