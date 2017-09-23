#pragma once
#include<thread> //thread
#include<atomic> //atomic_bool/int
#include<vector> //vector
#include<queue>
#include<functional>
#include<mutex>
#include<condition_variable>
#include<future>
#include<iostream>

using namespace std;
namespace kiton{

    class noncopy{//禁止拷贝和赋值拷贝
    protected:
        noncopy()= default;
        ~noncopy() = default;
        noncopy(noncopy&) = delete;
        noncopy&operator=(noncopy&) = delete;
    };
    //默认线程数取，机器的线程数
    const unsigned int DEFAULT_THREAD_NUM=thread::hardware_concurrency();
    class ThreadPool: protected noncopy{
        using Task =function<void(void)>;
    public:
        enum RUN_MODE{AT_ONCE=1,DELAY=0};//工作模式：立刻生产，延迟生产。。。
    private://val
        atomic<short> _mode;//线程池运行模式
        vector<thread> _pool;//线程池，线程池里面放的是生产者，主线程是消费者
        queue<Task> _task_que;//任务队列
        atomic_bool  _stop_add_task;//是否允许生产任务
        atomic_int  _free_thread_num;//线程数
        mutex       _lock;//互斥锁
        condition_variable _cv_queue_empty;//任务就绪变量

    public://fun
        //返回空闲线程数
        int free_count(){return _free_thread_num;}
        //返回池的大小
        int size(){return _pool.size();}
        //启动池工作
        bool start();
        ~ThreadPool();
        //初始化线程池初始数量，并且允许线程池工作
        ThreadPool(short mode=RUN_MODE::AT_ONCE,int thread_num=DEFAULT_THREAD_NUM):
                _stop_add_task(false),_free_thread_num(0),_mode(mode){
            init_pool(thread_num);
        }
        //任务提交，消费者线程调用
        template<class Fun,class...Args>
        auto add_task(Fun&& f,Args&&...args)->future<decltype(f(args...))>
        {
            if(_stop_add_task) throw runtime_error("现在禁止提交任务\n");
            using RetType = decltype(f(args...));//返回类型
            //绑定一个f(args...)调用对象
            function<RetType()> call=std::bind(forward<Fun>(f),forward<Args>(args)...);
            //生成类型为RetType()的函数packaged_task实体,packaged_task的任务为f(args...)
            auto task = make_shared<packaged_task<RetType()>>(
                    std::move(call)
            );//生成一个返回值类型是RetType,通过bind生成的可调用对象的指针
            auto res = task->get_future();//获得future
            {
                lock_guard<mutex> locker{_lock};//上锁
                _task_que.emplace([task]{
                    (*task)();
                });//任务队列里面放入一个执行void(void)匿名函数，该匿名函数执行(*task)()，
                // (*task)()里面才真正执行我们的任务,C++你赢了...
            }
            if(_mode==RUN_MODE::AT_ONCE){
                _cv_queue_empty.notify_one();//唤醒一个线程进行生产
            }
            return res;//返回该任务的future
        };

    private://fun
            void init_pool(int thread_num);
    };
    //初始化池
    void ThreadPool::init_pool(int thread_num){
        while((--thread_num)>=0)
        {
            function<void(void)> producer=[this]{
                //如果没有往线程池里面添加任务，或者任务队列非空
                while(!_stop_add_task||!_task_que.empty())
                {
                    Task task;
                    {
                        unique_lock<mutex> locker{_lock};
                        //再次检查任务队列是否有任务，空的话阻塞到有任务或者
                        _cv_queue_empty.wait(locker,[this]{return !_task_que.empty();});
                        //取出一个任务
                        //再次检查，走出来后如果关闭提交任务了或者任务队列空了，就返回
                        if(_task_que.empty()&&_stop_add_task) break;
                        //取出一个任务
                        task = std::move(_task_que.front());
                        _task_que.pop();
                        //调整空闲线程数
                    }
                    _free_thread_num--;
                    task();//执行任务
                    _free_thread_num++;
                }
            };
            //把一个生产者线程添加至，线程池里面
            _pool.emplace_back(producer);
            //空闲线程加1
            _free_thread_num++;
        }
    }

    bool ThreadPool::start()
    {/*
       线程池开足马力工作
       以后多线程add_task场景下，可能出现return之前被添加任务，
       导致任务队列还有任务没进行生产的情况。
       所以池开始工作后，需要切换成每添加一个任务就立即唤醒一个线程生产工作模式
       return处时插入的新任务会被立刻执行*/
        _mode=RUN_MODE ::AT_ONCE;
        _cv_queue_empty.notify_all();//唤醒所有线程
        return _task_que.empty();
    }

    ThreadPool::~ThreadPool()
    {
        _stop_add_task=true;

        if(_mode==RUN_MODE::AT_ONCE&&_task_que.empty()){
            for(auto& thread:_pool) {
                thread.detach();
            }
        }//立刻生产则，等待
        else if(_mode ==RUN_MODE::DELAY){
            _cv_queue_empty.notify_all();//唤醒所有线程
            for(auto& thread:_pool) {
                if(thread.joinable()) thread.join();
            }
        }
    }
}


