#include<iostream>
#include"ThreadPool.h"
#include<set>

int main(int argv,char** argc)
{
    kiton::ThreadPool threadPool;
    vector<future<void>>  result;
    set<thread::id> thread_id;
    for(int i=0;i<100;i++)
    {
        thread_id.insert(this_thread::get_id());
        result.emplace_back(
                threadPool.add_task([i,&thread_id]{cout<<this_thread::get_id()<<" task result:"<<i<<endl;
                                       })
        );
        result.emplace_back(
                threadPool.add_task([i,&thread_id]{
                    thread_id.insert(this_thread::get_id());
                    cout<<"thread id:"<<this_thread::get_id()<<endl;
                })
        );
    }

    for(auto& res:result){
        res.get();
    }
    cout<<endl<<thread_id.size()<<endl;
    return 0;
}
