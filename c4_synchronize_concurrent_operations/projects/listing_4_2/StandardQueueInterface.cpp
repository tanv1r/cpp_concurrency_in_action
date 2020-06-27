#include <iostream>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <queue>

using namespace std;

/*template <class T, class Container = deque<T>>
class MyQueue
{
    public:
        explicit MyQueue(const Container &);
        explicit MyQueue(Container && = Container());

        template <class Alloc> explicit MyQueue(Alloc const &);
        template <class Alloc> explicit MyQueue(Container const &, Alloc const &);
        template <class Alloc> explicit MyQueue(Container &&, Alloc const &);
        template <class Alloc> explicit MyQueue(MyQueue &&, Alloc const &);

        void swap(MyQueue & q);

        bool empty() const;
        size_type size() const;

        // To make it thread-safe we need to merge front and pop
        T & front();
        T const & front() const;
        T & back();
        T const & back() const;

        void push(T const & x);
        void push(T && x);
        void pop();
        template <class... Args> void emplace(Args &&... args);
};*/

template <typename T>
class ThreadSafeQueue
{
    public:
        ThreadSafeQueue()
        {
        }

        ThreadSafeQueue(ThreadSafeQueue const & other)
        {
            lock_guard<mutex> lk(otherQ.mut);
            dataQ = other.dataQ;
        }

        ThreadSafeQueue & operator=(ThreadSafeQueue const &) = delete;

        void push(T newValue)
        {
            lock_guard<mutex> lk(mut);
            dataQ.push(newValue);
            dataC.notify_one();
        }

        // Assumes T supports assignment.
        bool tryPop(T & value)
        {
            lock_guard<mutex> lk(mut);
            if (dataQ.empty())
            {
                return false;
            }

            value = dataQ.front();
            dataQ.pop();
            return true;
        }

        shared_ptr<T> tryPop()
        {
            lock_guard<mutex> lk(mut);
            if (dataQ.empty())
            {
                return shared_ptr<T>();
            }

            shared_ptr<T> valueSPtr(make_shared<T>(dataQ.front()));
            dataQ.pop();
            return valueSPtr;
        }

        void waitAndPop(T & value)
        {
            unique_lock<mutex> lk(mut);

            // before put to sleep the unique_lock is unlocked
            // when woken up, the thread is given back the lock
            //
            dataC.wait(
                lk,
                [this] // need to capture this to be able to check dataQ
                {
                    return !dataQ.empty();
                }
            );

            value = dataQ.front();
            dataQ.pop();
        }


        shared_ptr<T> waitAndPop()
        {
            unique_lock<mutex> lk(mut);
            dataC.wait(
                lk,
                [this] // need to capture this to be able to check dataQ
                {
                    return !dataQ.empty();
                }
            );

            shared_ptr<T> valueSPtr(make_shared<T>(dataQ.front()));
            dataQ.pop();
        }

        bool empty() const
        {
            lock_guard<mutex> lk(mut);
            return dataQ.empty();
        }

    private:
        // locking a mutex is a mutating operation
        // since we declared empty() as const, we
        // must now declare the mutex as mutable to be
        // able to lock it in empty()
        //
        mutable mutex mut;
        queue<T> dataQ;
        condition_variable dataC;
};

ThreadSafeQueue<int> safeQ;

void produce()
{
    while (true)
    {
        if (!safeQ.empty())
        {
            cout << "Producer going to fall asleep..." << endl;
            this_thread::sleep_for(chrono::milliseconds(rand()%10));
        }

        int value = rand()%100;
        cout << "Pushing " << value << endl;
        safeQ.push(value);
    }
}

void consume()
{
    while (true)
    {
        int value;
        cout << "Consumer calling wait and pop..." << endl;
        safeQ.waitAndPop(value);
        cout << "Consumer popped " << value << endl;
    }
}

int main()
{
    thread tP(produce), tC(consume);
    tP.join();
    tC.join();

    return 0;
}