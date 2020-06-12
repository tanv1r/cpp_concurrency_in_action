#include <iostream>
#include <thread>

using namespace std;

class ThreadGuard
{
    public:
        explicit ThreadGuard(thread & t)
            : thread_(t)
        {
        }

        ~ThreadGuard()
        {
            cout << "~ThreadGuard()" << endl;
            if (thread_.joinable()) 
            {
                thread_.join();
            }
        }

        ThreadGuard(ThreadGuard const &) = delete;
        ThreadGuard& operator=(ThreadGuard const &) = delete;

    private:
        thread & thread_;
};

struct func
{
    public:
        func(int iterCount)
            : iterCount_(iterCount)
        {          
        }

        void operator() ()
        {
            for (int i = 0; i < iterCount_; ++i)
            {
                cout << i << endl;
            }
        }

    private:
        int iterCount_ = 0;
};

int main()
{
    int local = 10;
    func myFunc{local};
    thread t{myFunc};

    // When main() tries to exit, myGuard goes out of scope
    // myGuard's destructor is called and join is called on the
    // started thread t, thus we do not need to remember to join.
    // 
    ThreadGuard myGuard{t};

    cout << "exiting main()" << endl;
}