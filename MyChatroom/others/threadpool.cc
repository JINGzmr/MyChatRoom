#include "head.h"
#include "threadpool.h"

using namespace std;

// 任务队列
TaskQueue::TaskQueue()
{
    pthread_mutex_init(&m_mutex, NULL);
}

TaskQueue::~TaskQueue()
{
    pthread_mutex_destroy(&m_mutex);
}
// 向任务队列中添加一个任务
void TaskQueue::addTask(Task &task)
{
    pthread_mutex_lock(&m_mutex);   // 让线程先后地访问任务队列，所以先加上一把锁
    m_queue.push(task);             // 一步到位，向任务队列加任务
    pthread_mutex_unlock(&m_mutex); // 任务结束，把锁解开
}
// 向任务队列中添加一个任务，使用函数指针和参数指针
void TaskQueue::addTask(callback func, void *arg)
{
    pthread_mutex_lock(&m_mutex);
    Task task;
    task.function = func;
    task.arg = arg;
    m_queue.push(task);
    pthread_mutex_unlock(&m_mutex);
}
// 从任务队列中取出一个任务
Task TaskQueue::takeTask()
{
    Task t;
    pthread_mutex_lock(&m_mutex); // 取任务的操作也要加锁
    if (m_queue.size() > 0)
    {
        t = m_queue.front(); // 从队头取出一个元素
        m_queue.pop();       // 将队头的元素弹出
    }
    pthread_mutex_unlock(&m_mutex);
    return t;
}

/*--------------------------------------------------------*/
// 线程池

// 构造函数，初始化线程池
ThreadPool::ThreadPool(int num)
{
    shutdown = 0;
    // 实例化任务队列
    m_TaskQ = new TaskQueue;
    do
    {
        // 初始化线程池
        threadNUM = num;
        // 初始化互斥锁,条件变量
        if (pthread_mutex_init(&mutexpool, NULL) != 0 ||
            pthread_cond_init(&notempty, NULL) != 0)
        {
            cout << "init mutex or condition fail..." << endl;
            break;
        }

        // 根据线程数, 创建线程
        for (int i = 0; i < threadNUM; ++i)
        {
            pthread_t tid;
            pthread_create(&tid, NULL, worker, this);
            pthread_detach(tid);
        }

    } while (0);
}
// 析构函数，销毁线程池
ThreadPool::~ThreadPool()
{
    shutdown = 1;

    // 唤醒所有消费者线程
    for (int i = 0; i < threadNUM; ++i)
    {
        pthread_cond_signal(&notempty);
    }
    // 销毁任务队列
    if (m_TaskQ)
        delete m_TaskQ;
    pthread_mutex_destroy(&mutexpool);
    pthread_cond_destroy(&notempty);
}

// 往任务队列里面添加任务,当任务添加成后，唤醒线程，来拿任务
void ThreadPool::addTask(Task task)
{
    if (shutdown)
    {
        return;
    }
    // 添加任务，不需要加锁，任务队列中有锁
    m_TaskQ->addTask(task);
    // 唤醒工作的线程
    pthread_cond_signal(&notempty);
}

// 工作线程任务函数（不断从任务队列中取出任务并执行，有锁）
void *ThreadPool::worker(void *arg)
{
    ThreadPool *pool = static_cast<ThreadPool *>(arg);
    // 一直不停的工作
    while (true)
    {
        // 访问任务队列(共享资源)加锁 线程池加锁
        pthread_mutex_lock(&pool->mutexpool);

        // 判断任务队列是否为空, 如果为空 工作线程阻塞
        while (pool->m_TaskQ->taskNumber() == 0 && pool->shutdown == 0)
        {
            cout << "thread waiting..." << endl;
            // 阻塞线程
            pthread_cond_wait(&pool->notempty, &pool->mutexpool);
        }

        // 判断线程池是否被关闭了
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&pool->mutexpool);
            pthread_exit(NULL);
        }

        //  从任务队列中取出一个任务
        Task task = pool->m_TaskQ->takeTask();
        // 线程池解锁
        pthread_mutex_unlock(&pool->mutexpool);

        //  执行任务
        cout << "thread start working..." << endl;
        // 调用任务函数，在执行任务时，该线程一直阻塞，直到任务完成*************（工作线程执行任务：怎么执行的，不同的事件要完成的任务不是不同吗？子线程怎么执行任务？）
        task.function(task.arg);

        // 任务处理结束
        cout << "thread end working...";

        //返回该线程到线程池
        
    }

    return nullptr;
}
