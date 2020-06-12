#include <algorithm>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

void hello(int i)
{
    cout << "hello there " << i << endl;
}

int main()
{
    vector<thread> mythreads;
    for (unsigned i = 0; i < 10; ++i)
    {
        mythreads.push_back( thread{hello, i} );
    }

    // mem_fn is wrapping up thread::join() in an object
    for_each( mythreads.begin(), mythreads.end(), mem_fn(&thread::join) );
}