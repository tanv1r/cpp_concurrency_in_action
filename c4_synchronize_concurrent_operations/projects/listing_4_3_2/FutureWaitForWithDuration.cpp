#include <iostream>
#include <chrono>
#include <thread>
#include <future>

using namespace std;
using namespace chrono;

void DoSomething()
{
    cout << "DoSomething() - " << this_thread::get_id() << endl;
    //this_thread::sleep_for(seconds(10));
    for (int i = 1; i < 3; ++i)
    {
        cout << i << endl;
        this_thread::sleep_for(seconds(1));
    }
}

int main()
{
    cout << "main() - " << this_thread::get_id() << endl;
    auto f = async(launch::async, DoSomething);

    auto status = f.wait_for(seconds(10));

    // if ready, you can access result with get()

    switch (status)
    {
        case future_status::ready:
        {
            cout << "Got result." << endl;
            break;
        }

        case future_status::deferred:
        {
            cout << "Deferred." << endl;
            break;
        }

        case future_status::timeout:
        {
            cout << "Timed out." << endl;
            break;
        }

        default:
        {
            cout << "Unknown status." << endl;
        }
    };
    

    return 0;
}