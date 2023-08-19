// 用于实现线程池中任务的加入与取出
// Reactor线程池模型 的一部分

#ifndef __THREADPOOL__
#define __THREADPOOL__
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <pthread.h>
#include <queue>
#include <unistd.h>
using namespace std;
using callback = void (*)(void *);

// 任务结构体（表示一个任务，包含一个函数指针和一个参数指针）
struct Task
{
    Task()
    {
        function = nullptr;
        arg = nullptr;
    }
    Task(callback f, void *arg)
    {
        function = f;
        this->arg = arg;
    }
    callback function;
    void *arg;
};

// 任务队列（用于存储待执行的任务）
class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    // 添加任务
    void addTask(Task &task);
    void addTask(callback func, void *arg);

    // 取出一个任务
    Task takeTask();

    // 获取当前队列中任务个数
    inline int taskNumber()
    {
        return m_queue.size();
    }

private:
    pthread_mutex_t m_mutex;  // 互斥锁
    std::queue<Task> m_queue; // 任务队列
};

// 线程池，管理工作线程和任务队列
class ThreadPool
{
public:
    ThreadPool(int num);
    ~ThreadPool();
    // 添加任务
    void addTask(Task task);

private:
    // 工作的线程的任务函数
    static void *worker(void *arg);

    // inline、static、extern .h need ,.cc not need
    // const .h.cc all need

private:
    TaskQueue *m_TaskQ;        // 任务队列
    int threadNUM;             // 线程数量
    int tasksize;              // 任务数量
    pthread_mutex_t mutexpool; // 锁整个线程池
    pthread_cond_t notempty;   // 任务队列是不是空
    int shutdown;              // 是不是要销毁线程池，销毁为1，不销毁为0
};

#endif
