#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "task.hpp"
#include "taskqueue.hpp"
#include <iostream>
#include <pthread.h>




//线程池类
template <typename T>
class threadpool{
public:

    //创建线程池并初始化
    threadpool(int minpthread, int maxpthread);
    //销毁线程池
    ~threadpool();
    //给线程池添加任务
    void addtask(task<T> task);
    //获取线程池中工作的线程个数
    int getbusynum();
    //获取线程池中活着的线程个数
    int getlivenum();

private:
    //工作线程的任务函数
    static void* worker(void* arg);
    //管理者线程任务函数
    static void* manager(void* arg);
    //单个线程退出
    void threadexit();


private:
    //任务队列
    taskqueue<T>* m_taskq;

    //管理者线程id
    pthread_t m_managerid;
    //工作线程id
    pthread_t *m_threadids;
    //最小线程数
    int m_minpthread;
    //最大线程数
    int m_maxpthread;
    //忙线程数
    int m_busypthread;
    //存活线程数
    int m_livepthread;
    //杀死的线程数
    int m_exitpthread;

    static const int NUMBER = 2;


    //锁整个线程池
    pthread_mutex_t m_mutexpool;
    //任务队列空了进行阻塞
    pthread_cond_t m_notempty;
 

    //是否销毁线程池，销毁为1，不销毁为0
    bool m_shutdown;


};



#endif