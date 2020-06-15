#include <deque>

template <typename T, typename Container=std::deque<T>>
class standardStack
{
    public:
        explicit standardStack(Container &);
        explicit standardStack(Container && = Container());
        template<class Alloc> explicit standardStack(Alloc const &);
        template<class Alloc> explicit standardStack(Container const &, Alloc const &);
        template<class Alloc> explicit standardStack(Container &&, Alloc const &);
        template<class Alloc> explicit standardStack(standardStack &&, Alloc const &);

        // Say all these operations are individually protected by mutex.
        bool empty() const;
        size_t size() const;
        T & top();
        T const & top() const;
        void push(T const &);
        void push(T &&);
        void pop();
        void swap(standardStack &&);
};

void doSomething(int value)
{
    // no-op
}

int main()
{
    standardStack<int> myStack;
    if (!myStack.empty())
    {
        int const value = myStack.top();
        myStack.pop();
        doSomething(value);
    }

    // RACE - 1
    // There is a single element (5) on myStack.
    // Two threads T1 and T2 are concurrently accessing myStack.
    // ------------------------------------------------------------------------------------------
    //         T1                                    |          T2
    // ------------------------------------------------------------------------------------------
    // if (!myStack.empty())                         |
    //{                                              | 
    //    // value == 5                              |
    //    int const value = myStack.top();           |
    //                                               |   if (!myStack.empty())
    //                                               |   {
    //                                               |       // value == 5
    //                                               |       int const value = myStack.top();
    //                                               |       // myStack is empty now
    //                                               |       myStack.pop();
    //                                               |       doSomething(value);
    //                                               |   }
    //    // ERROR: pop() called on an empty myStack |
    //    myStack.pop();                             |
    //}                                              |

    // RACE - 2
    //                                   |   |
    //                                   | 5 |
    //                                   | 4 |
    // There are two elements on myStack -----
    // Two threads T1 and T2 are concurrently accessing myStack.
    // ------------------------------------------------------------------------------------------
    //         T1                                    |          T2
    // ------------------------------------------------------------------------------------------
    // if (!myStack.empty())                         |
    // {                                             |
    //     // value == 5                             |
    //     int const value = myStack.top();          |        if (!myStack.empty())
    //                                               |        {
    //                                               |            // value == 5
    //                                               |            int const value = myStack.top();
    //                                               |            // pop 5
    //                                               |            myStack.pop();
    //                                               |            // process value == 5
    //                                               |            doSomething(value);
    //                                               |        }
    //     // pop 4                                  |
    //     myStack.pop();                            |  
    //     // process value == 5                     |
    //     // ERROR: 4 is silently dropped            |
    //     doSomething(value);                        |
    // }                                             |

    // The problem is in the standardStack interface. top() and pop() both should be under mutex.
    // - What if we eliminate top() and let pop() return the stack's top as well as remove it?
    // - If we do so, pop() could remove the top element from the stack and when memory is under pressure
    // - can fail at copy-construction of the top element to return. Especially if the stack is like
    // - myStack<vector<int>> where the vectors have lots of items.
    // 
    // - Option: 1
    // -- Pass in a reference
    // -- vector<int> result;
    // -- myStack.pop(result);
    // -- Problems:
    // --- (1) Caller may not have all relevant items to construct an element beforehand.
    // --- (2) Expensive.
    // --- (3) The type may not support assignment.
    // - Option: 2
    // -- Restrict usage to those types that support no-throw copy/move-ctor.
    // -- Problems:
    // --- (1) Limiting.
    // - Option: 3
    // -- Return a shared_ptr to the popped item. (Why not unique_ptr ?)
    // -- Problems:
    // --- For simple types like int, this could be overhead compared to just return a copy.
    // - Option: 4
    // -- Provide options 1 or 2, and 3.
}
