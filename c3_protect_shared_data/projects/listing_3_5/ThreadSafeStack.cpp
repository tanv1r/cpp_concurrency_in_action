#include <algorithm>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stack>
#include <thread>

struct EmptyStackException : std::exception
{
    const char * what() const noexcept
    {
        return "Attempt to perform operation on empty stack!";
    }
};

// A wrapper around standard stack to make it thread safe.
// top() has been merged into pop() and pop() now throws
// EmptyStackException. We could as well drop empty().
//
template<typename T>
class ThreadSafeStack
{
    public:
        ThreadSafeStack() {}
        ThreadSafeStack(ThreadSafeStack const & other)
        {
            // If the data_ is big we need to make sure lock on the mutex_
            // is held until copy completes. (WHY ?)
            //
            std::lock_guard<std::mutex>(mutex_);
            data_ = other.data_;
        }

        // Disallow assignment, goal is to limit update access to just push/pop
        ThreadSafeStack & operator=(ThreadSafeStack const &) = delete;

        void push(T newValue)
        {
            std::lock_guard<std::mutex> guard(mutex_);
            data_.push(newValue);
        }

        // Option - 3 from listing 3.3
        std::shared_ptr<T> pop()
        {
            std::lock_guard<std::mutex> guard(mutex_);

            if (data_.empty())
            {
                throw EmptyStackException();
            }

            // shared_ptr does not require copy, why not unique_ptr?
            std::shared_ptr<T> const stackTopSPtr(std::make_shared<T>(data_.top()));
            data_.pop();
            return stackTopSPtr;
        }

        // Option - 1 from listing 3.3
        // For simple types like int, this one would be more efficient than
        // constructing a shared_ptr
        //
        void pop(T & value)
        {
            std::lock_guard<std::mutex> guard(mutex_);
            if( data_.empty())
            {
                throw EmptyStackException();
            }

            // If data type does not allow assignment, we still can use shared_ptr option.
            value = data_.top();
            data_.pop();
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> guard(mutex_);
            return data_.empty();
        }

    private:
        std::stack<T> data_;
        mutable std::mutex mutex_;
};

// ThreadSafe stack to test.
ThreadSafeStack<int> integralStack;

// Two worker threads will be popping from the ThreadSafe stack and
// putting popped items in these separate vectors.
//
std::vector<int> poppedItems1, poppedItems2;

// A single thread will push a fixed number of items to the ThreadSafe stack.
void pushData(std::vector<int> const & input)
{
    try
    {
        for (int x : input)
        {
            integralStack.push(x);
        }
    }
    catch (std::exception & excp)
    {
        std::cerr << excp.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
}

// we shall divide items to be popped evenly between two threads
// to create contention. Even numbered one gets half, odd number one gets the other half.
void popData(int n, int id)
{
    auto & poppedItems = (id%2==1) ? poppedItems1 : poppedItems2;

    while (poppedItems.size() < n)
    {
        try
        {
            poppedItems.push_back(*integralStack.pop());
        }
        catch (std::exception const & e)
        {
            std::cerr << e.what() << '\n';
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        }
    }
}

int main()
{
    int elementCount = 10000;
    std::vector<int> inputVector;
    inputVector.reserve(elementCount);
    for (int i = 0; i < elementCount; ++i)
    {
        inputVector.push_back( i );
    }

    // Crate the three threads: 1 to push and 2 to pop
    std::vector<std::thread> workers;
    workers.push_back(std::thread(pushData, inputVector));
    workers.push_back(std::thread(popData, elementCount/2, 0));
    workers.push_back(std::thread(popData, elementCount/2, 1));

    // Wait for the threads to exit
    std::for_each(workers.begin(), workers.end(), std::mem_fn(&std::thread::join));

    std::cout << "pi1.size=" << poppedItems1.size() << ". pi2.size=" << poppedItems2.size() << std::endl;

    // validate that sum of the two poppedItems vector has exactly the same elements as in the inputVector
    // perhaps in a different order.
    //
    std::vector<int> allPoppedItems;
    allPoppedItems.reserve(elementCount);

    for (int a : poppedItems1)
    {
        if (std::find (allPoppedItems.cbegin(), allPoppedItems.cend(), a) == allPoppedItems.cend())
        {
            allPoppedItems.push_back(a);
        }
    }

    for (int a : poppedItems2)
    {
        if (std::find (allPoppedItems.cbegin(), allPoppedItems.cend(), a) == allPoppedItems.cend())
        {
            allPoppedItems.push_back(a);
        }
    }

    // match with input
    if (inputVector.size() != allPoppedItems.size())
    {
        std::cerr << "Size did not match!" << ", inputSize=" << inputVector.size() << ", allPoppedSize=" << allPoppedItems.size() << std::endl;
        return 1;
    }

    for (int x : inputVector)
    {
        if (std::find(allPoppedItems.cbegin(), allPoppedItems.cend(), x) == allPoppedItems.cend())
        {
            std::cerr << "Element missing" << std::endl;
        }
    }

    std::cout << "ALL GOOD!" << std::endl;
}