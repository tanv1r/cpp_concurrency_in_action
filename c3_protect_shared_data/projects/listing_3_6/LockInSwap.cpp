#include <mutex>

using namespace std;

class SomeBigObject
{
};

void swap(SomeBigObject & lhs, SomeBigObject & rhs);

class X
{
    public:
        X(SomeBigObject const & sd) : someDetails_(sd)
        {
        }

        friend void swap(X & lhs, X & rhs)
        {
            // If same instance, lock() on line 26 tries to acquire
            // lock on the same mutex twice which is undefined behavior.
            //
            if (&lhs == &rhs)
            {
                return;
            }

            // It can lock N number of mutexes at once, not just two
            // However, if there is an exception while acquiring
            // i-th lock, lock() will unlock on all (i-1) mutexes it
            // previously acquired lock for and then will rethrow.
            // So, better use try/catch
            //
            lock(lhs.detailsMutex_, rhs.detailsMutex_);

            // adopt_lock let lock_guard know that the mutex passed into its
            // ctor has already been locked, so just adopt ownership, do not
            // try locking on the mutex again
            //
            lock_guard<mutex> guardLhsMutex(lhs.detailsMutex_, adopt_lock);
            lock_guard<mutex> guardRhsMutex(rhs.detailsMutex_, adopt_lock);
            
            swap(lhs.someDetails_, rhs.someDetails_);
        }

    private:
        SomeBigObject someDetails_;
        mutex detailsMutex_;
};