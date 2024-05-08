#pragma once

#include "Timestamp.h"
#include "noncopyable.h"
#include "CurrentThread.cpp"

#include <memory>
#include <functional>
#include <atomic>
#include <mutex>

class Channel;
class Poller;

/**
 * @brief 时间循环类  主要包含了两个大模块 Channel   Poller（epoll的抽象）
 */
class EventLoop : noncopyable
{

public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

private:
    using ChannelList = std::vector<Channel *>;

    // 原子操作、通过CAS实现的

    std::atomic_bool looping_;                // 是否正在执行looping
    std::atomic_bool quit_;                   // 是否退出loop循环
    std::atomic_bool callingPendingFunctors_; // 标识当前loop是否有需要执行的回调操作

    const pid_t threadId_; // 记录当前loop所在线程的ID

    Timestamp pollReturnTime_; // poller返回事件的channels的时间戳
    std::unique_ptr<Poller> poller_;

    int wakeupfd_; // 主要作用，当mainLoop获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    std::vector<Functor> pendingFunctors_; // 存储loop需要执行的所有回调操作
    std::mutex mutex_;                     // 互斥锁，用来保护上面vector容器的线程安全操作

public:
    void loop();
    void quit();

    Timestamp pollReturnTime() const;

    // 在当前loop中执行cb
    void runInLoop(Functor cb);
    // 把cb放入队列中、唤醒loop所在的线程、执行回调
    void queueInLoop(Functor cb);
    void wakeup();
    void updatChannel(Channel *channel);
    void removeChannel(Channel *channel);
    void hasChannel(Channel *channel);
    bool isInLoopThread() const;

private:
    void doPendingFunctors();
    void handleRead();
};
