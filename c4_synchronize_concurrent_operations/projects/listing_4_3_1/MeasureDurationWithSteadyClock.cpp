#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

int main()
{
    steady_clock::time_point t1 = steady_clock::now();

    cout << "Printing 1000 stars..." << endl;
    for (int i = 1; i <= 1000; ++i)
    {
        cout << "*";
        this_thread::sleep_for(chrono::milliseconds(1));
    }

    cout << endl;

    steady_clock::time_point t2 = steady_clock::now();

    duration<double> timeSpan = duration_cast<duration<double>>(t2-t1);

    cout << "It took me " << timeSpan.count() << " seconds." << endl;

    return 0;
}