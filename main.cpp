#include<iostream>
#include"ThreadPool.h"
#include<set>
using namespace kiton;
int main(int argv,char** argc)
{
  
    vector<future<void>>  result;
    set<thread::id> thread_id;
    mutex lock;
    ThreadPool threadPool(ThreadPool::RUN_MODE::AT_ONCE,4);
        for (long long i = 0; i < 1000000; i++) {
            result.emplace_back(
                    threadPool.add_task([i, &thread_id,&lock] {
                        {
                            lock_guard<mutex> locker{lock};
                            thread_id.insert(this_thread::get_id());
                            cout <<"thread_id: "<< this_thread::get_id() << " run task result:" << i << endl;
                        }
                    })
            );
        }
   // threadPool.start();

    for(auto& res:result){
        res.get();
    }

    cout<<"池子的大小:"<<threadPool.size()<<endl;
    cout<<"空闲线程数:"<<threadPool.free_count()<<endl;
    cout<<"活跃线程数:"<<thread_id.size()<<endl;
   /*
    cout<<"活跃线程:"<<endl;
    for(auto& tid:thread_id){
        cout<<tid<<endl;
    }
    */
    return EXIT_SUCCESS;
}
