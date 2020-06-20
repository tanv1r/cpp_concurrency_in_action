#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

using ulong = unsigned long;

// Prevents deadlock by imposing strictly montonically decreasing
// order for lock acquiring for a thread
// Two threads cannot acquire lock on a mutex of same hierarchy value
// A single thread cannot acquire a lock with higher or equal hierarchy value
// that of the last mutex it acquired a lock on.
// Say mutexes are ordered like: a(10), b(9), c(8), d(7), e(6)
// Then for two threads T1 and T2, T1: a->b->e and T2: c->d is allowed
// concurrently.
//
class HierarchicalMutex
{
    public:
        explicit HierarchicalMutex(ulong value)
            : hierarchyValueOfMutex_(value)
            , previousHierarchyValueOfThread_(0)
        {
        }

        // Definition of lock, unlock, and tryLock makes
        // HierarchicalLock eligible to be used with lock_guard
        //
        void lock()
        {
            checkForHierarchyViolation();
            internalMutex_.lock();
            updateHierarchyValue();
        }

        void unlock()
        {
            thisThreadHierarchyValue_ = previousHierarchyValueOfThread_;
            internalMutex_.unlock();
        }

        bool tryLock()
        {
            checkForHierarchyViolation();
            if (!internalMutex_.try_lock())
            {
                return false;
            }

            updateHierarchyValue();
            return true;
        }

    private:
        void checkForHierarchyViolation()
        {
            if (thisThreadHierarchyValue_ <= hierarchyValueOfMutex_)
            {
                throw logic_error("mutex hierarchy violated");
            }
        }

        void updateHierarchyValue()
        {
            previousHierarchyValueOfThread_ = thisThreadHierarchyValue_;
            thisThreadHierarchyValue_ = hierarchyValueOfMutex_;
        }

        // Prevents two threads from locking the same HierarchicalMutex
        mutex internalMutex_;

        // Hierarchy value for the current mutex
        ulong const hierarchyValueOfMutex_;

        // Say a thread acquired 3 HierarchicalMutexes
        // in order of function calls m1(1000) -> m2 (100) -> m3(10)
        // Now if m3(10) is unlocked, the thread must have its hierarchy
        // value set to 100, so that it could acquire say mX(80) but cannot acquire
        // mY(200). That is the previousHierarchy value restores the acquiring
        // thread's hierarchy value appropriately.
        // 
        ulong previousHierarchyValueOfThread_;

        // This is the hierarchy value of the current thread
        // not of this HierarchicalMutex, it is like a global
        // variable for that thread
        //
        static thread_local ulong thisThreadHierarchyValue_;
};

// Each thread has an independent copy, initialized to max possible value
// So first time a thread can acquire any Hierarchical mutex if it is not
// already locked, however from second nested Hierarchical mutex onwards
// the hierarchy value must be strictly decreasing
//
thread_local ulong HierarchicalMutex::thisThreadHierarchyValue_(ULONG_MAX);

HierarchicalMutex highLevelMutex(10000);
HierarchicalMutex lowLevelMutex(5000);

int doLowLevelStuff()
{
    return 0;
}

int lowLevelFunc()
{
    lock_guard<HierarchicalMutex> lowGuard(lowLevelMutex);
    return doLowLevelStuff();
}

void doHighLevelStuff(int someParam)
{
    cout << "High level stuff: " << someParam << endl;
}

void highLevelFunc()
{
    lock_guard<HierarchicalMutex> highGuard(highLevelMutex);
    doHighLevelStuff(lowLevelFunc());
}

void threadA()
{
    highLevelFunc();
}

HierarchicalMutex otherMutex(100);

void doOtherStuff()
{
    cout << "Other stuff" << endl;
}

void otherStuff()
{
    highLevelFunc();
    doOtherStuff();
}

void threadB()
{
    lock_guard<HierarchicalMutex> otherGuard(otherMutex);
    otherStuff();
}

int main()
{
    // tA abides by the rule, first acquires high level lock
    // then in high level func, acquires low level lock, so it's fine
    //
    // thread tA(threadA);
    // tA.join();

    // tB violates rule, first locks low level, then tries to lock
    // high level lock thus gets logic_error at runtime.
    //
    thread tB(threadB);
    tB.join();
}