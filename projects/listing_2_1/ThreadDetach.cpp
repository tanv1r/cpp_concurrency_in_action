#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

struct func
{
    int & i_;
    func(int & i)
        : i_(i)
    {
    }

    void operator() ()
    {
        for (unsigned j = 0; j < 10; ++j) 
        {
            this_thread::sleep_for(chrono::seconds(1));
            cout << i_ << endl;
        }
    }
};

int main()
{
    int localX = 10;

    // Passing local variable as reference
    func myFunc(localX);

    // The argument to thread ctor does not need to be a function
    // it just needs to be callable, e.g. a class/func with () operator
    // works.
    //
    thread t(myFunc);

    // detach newly started thread from current thread, so main()'s thread
    // can exit while the myFunc one is still executing, what would happen if
    // my func tries to access the localX through the passed-in ref?
    // more immediately, once main exits, the process dies.
    //
    t.detach();
}