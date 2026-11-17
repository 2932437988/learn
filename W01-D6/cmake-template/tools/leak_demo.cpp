// 故意泄漏 4096 字节，供 GDB / valgrind / perf 取证。
// 编译：g++ -std=c++17 -O0 -g -fno-omit-frame-pointer tools/leak_demo.cpp -o
// build/leak_demo 运行：./build/leak_demo 1024
#include <cstddef>
#include <iostream>
#include <string>

static void leak_once(std::size_t n) {
  auto* first = new int[n];  // ← 故意不释放；函数返回后彻底失去指针
  auto* second = new int[n]; // ← 对照：正常释放

  for (std::size_t i = 0; i < n; ++i) {
    first[i] = 1;
    second[i] = 2;
  }

  delete[] second;
  // 修复演示：取消下一行注释，valgrind 的 definitely lost 应从 4096 变 0
  // delete[] first;
}

int main(int argc, char** argv) {
  const std::size_t n = (argc > 1) ? static_cast<std::size_t>(std::stoul(argv[1])) : 1024;
  leak_once(n);
  std::cout << "leak_once done, n = " << n << '\n';
  return 0;
}