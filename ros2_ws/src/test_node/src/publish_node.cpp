// 1. 核心头文件：ROS 2 基础API + 字符串消息类型
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

// 2. 自定义发布者节点类（继承 rclcpp::Node）
class PublisherNode : public rclcpp::Node
{
public:
    // 构造函数：初始化节点名 + 创建发布者 + 定时器
    PublisherNode() : Node("publisher_node"), count_(0)
    {
        // ========== 创建发布者核心逻辑 ==========
        // 参数1：话题名（必须唯一，订阅者需匹配）；参数2：QoS历史深度（保存最后10条消息）
        pub_ = this->create_publisher<std_msgs::msg::String>("chatter", 10);

        // 定时器：每1秒触发一次发布回调（模拟高频数据发送）
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&PublisherNode::publish_callback, this)
        );

        RCLCPP_INFO(this->get_logger(), "发布者节点启动，话题：chatter");
    }

private:
    // 发布回调函数：构造消息并发布
    void publish_callback()
    {
        // 构造 ROS 2 标准消息对象
        auto msg = std_msgs::msg::String();
        msg.data = "Hello ROS 2! Count: " + std::to_string(count_);
        
        // ========== 发布消息到话题 ==========
        pub_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "发布消息：%s", msg.data.c_str());
        count_++;
    }

    // 成员变量：发布者对象 + 定时器 + 计数
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    size_t count_;
};

// 3. 主函数：节点入口
int main(int argc, char * argv[])
{
    // 初始化 ROS 2 上下文（必须）
    rclcpp::init(argc, argv);
    // 创建节点实例（智能指针）
    auto node = std::make_shared<PublisherNode>();
    // 自旋节点（阻塞，处理定时器回调）
    rclcpp::spin(node);
    // 关闭 ROS 2 上下文
    rclcpp::shutdown();
    return 0;
}subscribe_node.cpp