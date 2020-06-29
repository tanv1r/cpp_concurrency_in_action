#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <utility>

using namespace std;

mutex m;
deque<packaged_task<void()>> tasks;

bool guiShutdownMessageReceived();
void getAndProcessGuiMessage();

void guiThread()
{
    while (!guiShutdownMessageReceived())
    {
        getAndProcessGuiMessage();
        packaged_task<void()> task;

        {
            // The mutex is protecting the tasks deque
            lock_guard<mutex> lk(m);

            // If no task has been posted yet, no-op
            if (tasks.empty())
            {
                continue;
            }

            task = move(tasks.front());
            tasks.pop_front();
        }
       
        task();
    }
}

thread guiBgThread(guiThread);

template<typename Func>
// Once posted, the caller can wait on the returned future.
future<void> postTaskForGuiThread(Func f)
{
    packaged_task<void()> task(f);
    auto resultFut = task.get_future();
    lock_guard<mutex> lk(m);
    tasks.push_back(move(task));
    return resultFut;
}