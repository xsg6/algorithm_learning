cmake_minimum_required(VERSION 3.8)
project(try_node)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 查找依赖包
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)  # 新增：依赖字符串消息类型（必须加！）

# ========== 发布者节点编译 ==========
add_executable(publisher_node src/publisher_node.cpp)
ament_target_dependencies(publisher_node rclcpp std_msgs)  # 补充 std_msgs 依赖
install(TARGETS
  publisher_node
  DESTINATION lib/${PROJECT_NAME}
)

# ========== 订阅者节点编译 ==========
add_executable(subscriber_node src/subscribe_node.cpp)
ament_target_dependencies(subscriber_node rclcpp std_msgs)  # 补充 std_msgs 依赖
install(TARGETS
  subscriber_node
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()