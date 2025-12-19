// 1. 核心头文件
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

// 2. 自定义订阅者节点类
class SubscriberNode : public rclcpp::Node
{
public:
    // 构造函数：初始化节点名 + 创建订阅者
    SubscriberNode() : Node("subscriber_node")
    {
        // ========== 创建订阅者核心逻辑 ==========
        // 参数1：话题名（必须和发布者一致）；参数2：QoS历史深度；参数3：消息回调函数
        sub_ = this->create_subscription<std_msgs::msg::String>(
            "chatter",
            10,
            std::bind(&SubscriberNode::sub_callback, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "订阅者节点启动，监听话题：chatter");
    }

private:
    // 订阅回调函数：收到话题消息时触发
    void sub_callback(const std_msgs::msg::String::SharedPtr msg) const
    {
        // 处理收到的消息（这里仅打印，可替换为点云滤波、运动控制等业务逻辑）
        RCLCPP_INFO(this->get_logger(), "收到话题消息：%s", msg->data.c_str());
    }

    // 成员变量：订阅者对象
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
};

// 3. 主函数
int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SubscriberNode>();
    rclcpp::spin(node); // 自旋等待话题消息
    rclcpp::shutdown();
    return 0;
}