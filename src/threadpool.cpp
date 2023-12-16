#include "threadpool.hpp"
#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
using namespace std;

//创建线程池并初始化
template <typename T>
threadpool<T>::threadpool(int minpthread, int maxpthread)
{
    do{
        //实例化任务队列
        m_taskq = new taskqueue;
        if(m_taskq == nullptr)
        {
            cout<<"malloc taskq fail...."<<endl;
            break;
        }
        //实例化线程数
        m_threadids = new pthread_t[maxpthread];
        if(m_threadids == nullptr)
        {
            cout<<"malloc threadids fail...."<<endl;
            break;
        }
        memset(m_threadids, 0, sizeof(pthread_t)* maxpthread);
        this->m_minpthread = minpthread;
        this->m_maxpthread = maxpthread;
        this->m_busypthread = 0;
        this->m_livepthread = minpthread;
        this->m_exitpthread = 0;

        if(pthread_mutex_init(&m_mutexpool, NULL) != 0 ||
        pthread_cond_init(&m_notempty, NULL) != 0)
        {
            printf("mutex or cond init fail...\n");
            break;
        }

        this->m_shutdown = false;

        //创建线程
        pthread_create(&m_managerid, NULL, manager, this);
        for(int i = 0; i < minpthread; ++i)
        {
            pthread_create(&m_threadids[i], NULL, worker, this);
        }
        return;

    }while(0);

    //释放资源
    if(m_threadids) delete [] m_threadids;
    if(m_taskq) delete m_taskq;
    

}
//析构函数
template <typename T>
threadpool<T>::~threadpool()
{
    //关闭线程池
    m_shutdown = true;
    //阻塞回收管理者线程
    pthread_join(m_managerid, NULL);
    //唤醒阻塞的消费者线程
    for(int i =0; i < m_livepthread; i++)
    {
        pthread_cond_signal(&m_notempty);
    }
    //释放内存
    if(m_taskq)
    {
        delete m_taskq;
    }
    if(m_threadids)
    {
        delete[] m_threadids;
    }

    pthread_mutex_destroy(&m_mutexpool);
    pthread_cond_destroy(&m_notempty);


}



//生产者线程的任务函数
template <typename T>
void* threadpool<T>::worker(void* arg)
{
    threadpool* pool = static_cast<threadpool*>(arg);
    while(1)
    {
        pthread_mutex_lock(&pool->m_mutexpool);
        while(pool->m_taskq->tasknumber() == 0 && !pool->m_shutdown)
        {
            //组塞工作线程
            pthread_cond_wait(&pool->m_notempty, &pool->m_mutexpool);
        }
        if(pool->m_exitpthread > 0)
        {
            pool->m_exitpthread--;     
            if(pool->m_livepthread > pool->m_minpthread)
            {
                pool->m_livepthread--;
                pthread_mutex_unlock(&pool->m_mutexpool);
                pool->threadexit();
            }
        }

        //判断线程池是否被关闭
        if(pool->m_shutdown)
        {
            pthread_mutex_unlock(&pool->m_mutexpool);
            pool->threadexit();
        }

        //从任务队列中取出一个任务
        task<T> task = pool->m_taskq->taketask();

        //解锁
        pool->m_busypthread++;
        pthread_mutex_unlock(&pool->m_mutexpool);
        cout<<"thread "<<to_string(pthread_self())<<" start working..\n";

        task.function(task.m_arg);
        delete task.m_arg;
        task.m_arg = nullptr;

        cout<<"thread "<<to_string(pthread_self())<<" start working..\n";

        pthread_mutex_lock(&pool->m_mutexpool);
        pool->m_busypthread--;
        pthread_mutex_unlock(&pool->m_mutexpool);
    }
    return NULL;
}




//管理者线程的任务函数
template <typename T>
void* threadpool<T>::manager(void* arg)
{
    threadpool* pool = static_cast<threadpool*> (arg);
    while(!pool->m_shutdown)
    {
        sleep(3);

        //取出线程池任务的数量和当前线程的数量
        pthread_mutex_lock(&pool->m_mutexpool);
        int queuesize = pool->m_taskq->tasknumber();
        int livenum = pool->m_livepthread;
        int busynum = pool->m_busypthread;
        pthread_mutex_unlock(&pool->m_mutexpool);            

        //添加线程
        //任务个数 > 存活线程数 && 存活线程数 < 最大线程数
        if(queuesize > livenum && livenum < pool->m_maxpthread)
        {
            pthread_mutex_lock(&pool->m_mutexpool);
            int counter = 0;
            for(int i = 0; i <pool->m_maxpthread && counter < NUMBER && pool->m_livepthread < pool->m_maxpthread; ++i)
            {
                if(pool->m_threadids[i] == 0)
                {
                    pthread_create(&pool->m_threadids[i], NULL, worker, pool);
                    counter++;
                    pool->m_livepthread++;
                }
            }
            
            pthread_mutex_unlock(&pool->m_mutexpool); 
        }


        //销毁线程
        if(busynum*2 < livenum && livenum > pool->m_minpthread)
        {
            pthread_mutex_lock(&pool->m_mutexpool);
            pool->m_exitpthread = NUMBER;
            pthread_mutex_unlock(&pool->m_mutexpool);
            for(int i = 0; i < NUMBER; ++i)
            {
                pthread_cond_signal(&pool->m_notempty);
            }
        }
    }

    return NULL;

}


//单个线程退出
template <typename T>
void threadpool<T>::threadexit()
{
    pthread_t tid = pthread_self();
    for(int i = 0; i < this->m_maxpthread; ++i)
    {
        if(this->m_threadids[i] == tid)
        {
            m_threadids[i] = 0;
            cout<<"threadexit() called,"<< to_string(tid)<<"exiting...\n";
            break;
        }
    }
    pthread_exit(NULL);
}

template <typename T>
void threadpool<T>::addtask(task<T> task)
{
    if(m_shutdown)
    {
        return;
    }

    //添加任务
    this->m_taskq->addtask(task);

    pthread_cond_signal(&m_notempty);
}    


//获取线程池中工作的线程个数
template <typename T>
int threadpool<T>::getbusynum()
{
    pthread_mutex_lock(&m_mutexpool);
    int busynum = m_busypthread;
    pthread_mutex_unlock(&m_mutexpool);
    return busynum;
}


//获取线程池中活着的线程个数
template <typename T>
int threadpool<T>::getlivenum()
{
    pthread_mutex_lock(&m_mutexpool);
    int livenum = m_busypthread;
    pthread_mutex_unlock(&m_mutexpool);
    return livenum;
}