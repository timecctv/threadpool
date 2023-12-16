#ifndef TASK_HPP
#define TASK_HPP

#include <iostream>
using callback = void (*)(void* arg);

//任务类
template <typename T>
class task{
    //构造函数
    // task();
    // task(callback f, void* arg);
    //构造任务类
public:
    task()
    {
        function = nullptr;
        m_arg = nullptr;
    }
    task(callback f, void* arg)
    {
        this->function = f;
        this->m_arg = (T*)arg;
    }

    //回调函数
    void (*function)(void* arg);
    //函数需要传入的参数
    T* m_arg;
};

#endif