add_executable(test_ring_buffer test_ring_buffer.cpp)
add_executable(test_config     test_config.cpp)

target_link_libraries(test_ring_buffer PRIVATE robot_template::core GTest::gtest_main)
target_link_libraries(test_config      PRIVATE robot_template::core GTest::gtest_main)

# sanitizer 构建时不能在 POST_BUILD 运行插桩过的二进制：
# TSan 会直接 FATAL 在 ASLR 布局自检（D4 6.5.1），ASan 也可能被 LSan 干扰。
# 改成 PRE_TEST：build 阶段不执行，列举用例推迟到 ctest 运行时。
if(ENABLE_ASAN OR ENABLE_UBSAN OR ENABLE_TSAN)
  set(RT_DISCOVERY_MODE PRE_TEST)
else()
  set(RT_DISCOVERY_MODE POST_BUILD)
endif()

include(GoogleTest)
gtest_discover_tests(test_ring_buffer DISCOVERY_MODE ${RT_DISCOVERY_MODE})
gtest_discover_tests(test_config      DISCOVERY_MODE ${RT_DISCOVERY_MODE})