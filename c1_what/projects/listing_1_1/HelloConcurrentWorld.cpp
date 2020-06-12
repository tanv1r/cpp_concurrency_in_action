#include <iostream>
#include <thread>

using namespace std;

void hello()
{
    cout << "Hello, Concurrent World!" << endl;
}

int main()
{
    // Creating thread object starts execution of the thread
    thread t(hello);

    // Block current thread from proceeding further until the newly started thread returns
    t.join();
}