#include <iostream>
#include <functional>
#include <thread>
#include <future>

using namespace std;

void printInteger(future<int> & fut)
{
    int x = fut.get();
    cout << "value: " << x << endl;
}

int main()
{
    promise<int> prom;
    auto fut = prom.get_future();

    thread t1(printInteger, ref(fut));

    // It is almost like condition variable
    // but it can communicate data.
    //
    prom.set_value(100);

    t1.join();

    return 0;
}