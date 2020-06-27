#include <iostream>
#include <chrono>
#include <future>
#include <thread>

using namespace std;

int answerToLife()
{
    this_thread::sleep_for(chrono::seconds(10));

    return -1;
}

int main()
{
    // future is like C# Task
    future<int> theAnswer = async(answerToLife);

    for (int i = 0; i < 10; ++i)
    {
        cout << "count " << i << endl;
    }

    // get() on future is like C#'s GetAwaiter().GetResult();
    cout << "Here goes the answer: " << theAnswer.get() << endl;

    return 0;
}