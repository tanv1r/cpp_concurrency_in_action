#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

class ProtectedData
{
    public:
        ProtectedData(unsigned x)
            : data_(x)
        {
        }

        template<typename Function>
        void processData(Function func)
        {
            // Even though we are protecting access to the data through mutex
            // that is not enough, the passed in func can steal the memory address
            // of the protected data and then freely manipulate bypassing lock.
            // We could prevent the stealing by applying the func on a copy.
            //
            lock_guard<mutex> funcGuard{ dataLock_ };

            // deep copy
            // auto copy = data_;

            func(data_);
            // func(copy);

            // copy assignment should be a deep copy as wellss
            // data_ = copy;
        }

        unsigned data()
        {
            return data_;
        }

    private:
        unsigned data_ = 0;
        mutex dataLock_;
};

unsigned * steal;

void badGuy(unsigned & takeit)
{
    // steal the memory address of the protected data
    steal = &takeit;
}

int main()
{
    ProtectedData protectedData(10);

    // prints 10
    cout << protectedData.data() << endl;

    protectedData.processData(badGuy);

    ++*steal;

    // prints 11
    cout << protectedData.data() << endl;
}

