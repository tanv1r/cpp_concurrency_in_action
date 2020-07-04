#include <future>
#include <iostream>
#include <thread>

using namespace std;

template<typename F, typename A>
future<result_of<F(A &&)>::type> SpawnTask(F && f, A && a)
{
    using resultType = result_of<F(A &&)>::type;

    packaged_task<resultType(A &&)> task(move(f));
    future<resultType> result(task.get_future());
    thread t(move(task), move(a));
    t.detach();

    return result;
}