#include <iostream>
#include <thread>

using namespace std;

class ScopedThread
{
    public:
        explicit ScopedThread(thread && t)
            : t_{move(t)}
        {
            if(!t_.joinable())
            {
                throw logic_error("No thread");
            }
        }

        ~ScopedThread()
        {
            // no need to check if it is joinable because validation happened in ctor
            t_.join();
        }

        // disable copying
        ScopedThread(ScopedThread const &) = delete;
        ScopedThread & operator=(ScopedThread const &) = delete;

    private:
        thread t_;
};

struct Func
{
    public:
        Func(int count, string const & message)
            : printCount_{count}
            , message_{message}
        {
        }

        void operator() () const
        {
            for (int i = 0; i < printCount_; ++i)
            {
                cout << i << ": " << message_ << endl;
            }
        }

    private:
        int printCount_{0};
        string message_;
};

int main()
{
    Func myFunc{10, "RobinHood"};

    // This is an improvement over ThreadGuard from listing_2_3
    // Becase here we do not even need to give the newly created
    // thread a name. But the basic join logic is using RAII.
    //
    ScopedThread st{thread{myFunc}};

    cout << "Exiting from main..." << endl;
}