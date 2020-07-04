#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <mutex>
#include <vector>
#include <algorithm>

using namespace std;
using namespace chrono;

timed_mutex mtx;

void FireWorks()
{
    auto const & tid = this_thread::get_id();

    while ( !mtx.try_lock_for(milliseconds(200)) )
    {
        cout << ".";
    }

    this_thread::sleep_for(seconds(1));
    cout << "*" << endl;

    mtx.unlock();
}

int main()
{
    vector<thread> threads;
    for (int i = 1; i <= 10; ++i)
    {
        threads.push_back( thread(FireWorks) );
    }

    for_each( threads.begin(), threads.end(), mem_fn(&thread::join) );
}

// Output of one run looks below. One thread will acquire the lock every second, so printing 10 stars
// by the 10 threads will take about 10 seconds.
// However, when for the first time, a thread got the lock, there were 9 other threads each trying
// to lock every 200 ms. Looks like multiple thread can concurrently try to get lock that is why there are more
// than 9 dots in the first line. Over time, later lines with lesser and lesser number of dots show that contention
// reduced gradually.
//
// ....................................*
// ........................................*
// ...................................*
// ..............................*
// .........................*
// ....................*
// ...............*
// ..........*
// ....*
// *