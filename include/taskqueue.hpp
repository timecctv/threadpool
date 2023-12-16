#ifndef TASKQUEUE_HPP
#define TASKQUEUE_HPP

#include "task.hpp"
#include <queue>
using callback = void (*)(void* arg);

//任务队列类
template <typename T>
class taskqueue{
public:
    taskqueue();
    ~taskqueue();

    //添加任务
    void addtask(task<T> task);
    void addtask(callback f, void* arg);
    //取出任务
    task<T> taketask();
    //获取当前任务的个数
    inline size_t tasknumber()
    {
        return m_taskq.size();
    }

private:
    pthread_mutex_t m_mutex;
    std::queue<task<T>> m_taskq;
};

#endif
