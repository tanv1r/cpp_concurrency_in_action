#include <chrono>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

mutex dataMutex;
queue<int> dataQueue;
condition_variable dataCond;

/*void consumer()
{
    unique_lock<mutex> mlock(dataMutex);

    while (true)
    {
        cout << "CONSMER ROUND" << endl;

        while (!dataQueue.empty())
        {
                int x = dataQueue.front();
                cout << "popping " << x << endl;
                dataQueue.pop();
        }

        cout << "gonna sleep ..." << endl;
        mlock.unlock();
        this_thread::sleep_for(chrono::seconds(rand()%10));   
        mlock.lock();
    }
}

void producer()
{
    unique_lock<mutex> mlock(dataMutex);

    while (true)
    {
        cout << "PRODUCER ROUND" << endl;

        if (dataQueue.empty())
        {
            int x = rand() % 100;
            cout << "pushing " << x << endl;
            dataQueue.push(x);
        }

        cout << "gonna sleep..." << endl;
        mlock.unlock();
        // Choosing the right amount of wait is difficult
        this_thread::sleep_for(chrono::seconds(rand()%10));
        mlock.lock();
    }
}*/

void consumer()
{
    while (true)
    {
        cout << "CONSUMER ROUND//" << endl;

        unique_lock<mutex> m(dataMutex);

        if (!dataQueue.empty())
        {
            int x = dataQueue.front();
            cout << "popping " << x << endl;
            dataQueue.pop();

            dataCond.notify_one();
        }

        // When it goes in wait, the supplied m is unlocked
        // and when it wakes up to check the condition it reacquires the lock
        //
        // Due to Spurious Wake, the thread actually can check
        // multiple times and find the condition false. The condition
        // check should not have side-effects.
        //
        dataCond.wait(
            m, 
            []{ return !dataQueue.empty();});
    }
}

void producer()
{
    while (true)
    {
        unique_lock<mutex> m(dataMutex);

        cout << "\\PRODUCER ROUND" << endl;
        if (dataQueue.empty())
        {
            int x = rand()%100;
            cout << "pushing " << x << endl;
            dataQueue.push(x);

            dataCond.notify_one();
        }

        dataCond.wait(
            m,
            []{return dataQueue.empty();});
    }
}


int main()
{
    thread p(producer);
    thread c(consumer);

    p.join();
    c.join();
}