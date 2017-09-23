#include<iostream>
#include"ThreadPool.h"
#include<set>
using namespace kiton;
int main(int argv,char** argc)
{

    vector<future<void>>  result;
    set<thread::id> thread_id;
    ThreadPool threadPool(ThreadPool::RUN_MODE::DELAY);
        for (int i = 0; i < 100; i++) {
            thread_id.insert(this_thread::get_id());
            result.emplace_back(
                    threadPool.add_task([i, &thread_id] {
                        cout <<"thread_id: "<< this_thread::get_id() << " run task result:" << i << endl;
                    })
            );
        }
    threadPool.start();

    for(auto& res:result){
        res.get();
    }

    cout<<endl<<thread_id.size()<<endl;
    return EXIT_SUCCESS;
}
