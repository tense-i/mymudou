#include "EventLoopThread.h"
#include "EventLoop.h"
#include <functional>
#include <string> // Add this line

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this), name),
      mutex_(),
      cond_(),
      callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    // 在这会并发调用下面的 threadFunc函数
    thread_.start(); // 启动底层的新线程

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr)
        {
            // 并发下、等待loop_对象创建完成
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

/**
 * @brief threadFunc函数会绑定到Thread类的对象中运行
 */
void EventLoopThread::threadFunc()
{
    // 这个EventLoop对象是在新线程中创建的，所以这个EventLoop对象和创建它的线程是一一对应的
    EventLoop loop; // 创建一个独立的eventloop，和上面的线程是一一对应的，one loop per thread

    // q:为什么要调用callback_函数？
    // a:因为callback_函数是在EventLoopThread对象创建时传入的，用于初始化EventLoop对象。
    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        // 唤醒startLoop函数、已经创建好了loop
        cond_.notify_one();
    }

    // 这里会阻塞，直到loop退出
    loop.loop();

    // 为什么loop_要置空？
    // 因为loop_是一个指针，指向的对象已经被销毁了，所以要置空。loop_指向的是一个栈上的对象（上面创建的）
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}
