#ifndef MONOTONIC_STACK_H
#define MONOTONIC_STACK_H

#include <stack>
#include <stdexcept>
#include <functional>
#include <utility>

template <typename T, typename Compare = std::less<T>>
class MonotonicStack {
public:
    explicit MonotonicStack(Compare comp = Compare()) : comp_(comp) {}

    // push by const ref
    void push(const T& value) {
        while (!data_.empty() && comp_(value, data_.top())) {
            data_.pop();
        }
        data_.push(value);
    }

    // push by rvalue
    void push(T&& value) {
        while (!data_.empty() && comp_(value, data_.top())) {
            data_.pop();
        }
        data_.push(std::move(value));
    }

    // emplace convenience
    template <typename... Args>
    void emplace(Args&&... args) {
        T value(std::forward<Args>(args)...);
        while (!data_.empty() && comp_(value, data_.top())) {
            data_.pop();
        }
        data_.push(std::move(value));
    }

    void pop() {
        if (data_.empty()) throw std::out_of_range("Pop from empty MonotonicStack");
        data_.pop();
    }

    const T& top() const {
        if (data_.empty()) throw std::out_of_range("Top from empty MonotonicStack");
        return data_.top();
    }

    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }

private:
    std::stack<T> data_;
    Compare comp_;
};

// 方便别名：单调增（底到顶非降）和单调减（底到顶非升）
template <typename T>
using MonotonicIncreasingStack = MonotonicStack<T, std::less<T>>;

template <typename T>
using MonotonicDecreasingStack = MonotonicStack<T, std::greater<T>>;

#endif // MONOTONIC_STACK_H