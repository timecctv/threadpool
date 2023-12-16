#include "task.hpp"
#include "taskqueue.hpp"



    template <typename T>
    taskqueue<T>::taskqueue()
    {
        pthread_mutex_init(&m_mutex, NULL);
    }
    template <typename T>
    taskqueue<T>::~taskqueue()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    //添加任务
    template <typename T>
    void taskqueue<T>::addtask(task<T> task)
    {
        pthread_mutex_lock(&m_mutex);
        m_taskq.push(task);
        pthread_mutex_unlock(&m_mutex);
    }
    template <typename T>
    void taskqueue<T>::addtask(callback f, void* arg)
    {
        pthread_mutex_lock(&m_mutex);
        m_taskq.push(task(f,arg));
        pthread_mutex_unlock(&m_mutex);
    }
    //取出任务
    template <typename T>
    task<T> taskqueue<T>::taketask()
    {
        task<T> t;
        pthread_mutex_lock(&m_mutex);
        if(!m_taskq.empty())
        {
            t = m_taskq.front();
            m_taskq.pop();
        }
        pthread_mutex_unlock(&m_mutex);
        return t;
    }
    //获取当前任务的个数
    template <typename T>
    inline size_t taskqueue<T>::tasknumber()
    {
        return m_taskq.size();
    }
