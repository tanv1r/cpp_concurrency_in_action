#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

mutex globalCountLock;
int globalCount = 0;

// With high number of increments and threads, it is almost certain to give incorrect total
// ~6 ns per increment
//
void increaseWithoutLock(unsigned localCount)
{
    while (localCount-- > 0)
    {
        ++globalCount;
    }
}

// This one seems to perform the best, but remembering to unlock is a hassle
// ~3 ns per increment
void increaseWithFunctionLevelMutex(unsigned localCount)
{
    globalCountLock.lock();

    while (localCount-- > 0)
    {
        ++globalCount;
    }

    globalCountLock.unlock();
}

// More usable than plain function level mutex
// ~3 ns per increment
void increaseWithFunctionLevelGuradedMutex(unsigned localCount)
{
    lock_guard<mutex> guardedLock{ globalCountLock };

    while (localCount-- > 0)
    {
        ++globalCount;
    }
}

// This is much worse in terms of performance, perhaps locking at too granular a level is not good
// ~60 ns
void increaseWithLoadModifyUpdateLevelMutex(unsigned localCount)
{
    while (localCount-- > 0)
    {
        globalCountLock.lock();

        ++globalCount;

        globalCountLock.unlock();
    }
}

// similar to the plain mutex at update level
void increaseWithLoadModifyUpdateLevelGuardedMutex(unsigned localCount)
{
    while (localCount-- > 0)
    {
        lock_guard<mutex> guardedLock{ globalCountLock };

        ++globalCount;
    }
}

void performActionWithThreads(
    function<void(unsigned)> action,
    unsigned workerCount,
    unsigned increasePerWorker
)
{
    globalCount = 0;

    vector<thread> workers;

    for (unsigned i = 0; i < workerCount; ++i)
    {
        workers.push_back(
            thread(action, increasePerWorker)
        );
    }

    for_each(workers.begin(), workers.end(), mem_fn(&thread::join));
}

double measureAverageActionTime(
    function<void(unsigned)> action,
    unsigned workerCount,
    unsigned increasePerWorker,
    unsigned iterCount)
{
    auto beginTime = chrono::high_resolution_clock::now();

    while (iterCount-- > 0)
    {
        performActionWithThreads(action, workerCount, increasePerWorker);
    }

    auto finishTime = chrono::high_resolution_clock::now();

    auto elapsedTotalSeconds = (finishTime - beginTime).count()/1e9;

    return elapsedTotalSeconds/iterCount;
}

int main()
{
    unsigned workerCount = thread::hardware_concurrency()-1;
    cout << "workerCount = " << workerCount << endl;
    unsigned increasePerWorker = 1000000;
    unsigned iterationCount = 1000;

    auto elapsedSecondsWithoutLock
        = measureAverageActionTime(
            increaseWithoutLock,
            workerCount,
            increasePerWorker,
            iterationCount
        );

    cout << "Avg. time (w/o lock): " << elapsedSecondsWithoutLock << " seconds." << endl;

    auto elapsedSecondsWithFuncMutex
        = measureAverageActionTime(
            increaseWithFunctionLevelMutex,
            workerCount,
            increasePerWorker,
            iterationCount
        );

    cout << "Avg. time (w/ fnc mutex): " << elapsedSecondsWithFuncMutex << " seconds." << endl;

    auto elapsedSecondsWithFuncGuard
        = measureAverageActionTime(
            increaseWithFunctionLevelGuradedMutex,
            workerCount,
            increasePerWorker,
            iterationCount
        );

    cout << "Avg. time (w/ fnc guard): " << elapsedSecondsWithFuncGuard << " seconds." << endl;

    auto elapsedSecondsLmuMutex
        = measureAverageActionTime(
            increaseWithLoadModifyUpdateLevelMutex,
            workerCount,
            increasePerWorker,
            iterationCount
        );

    cout << "Avg. time (w/ lmu mutex): " << elapsedSecondsLmuMutex << " seconds." << endl;

    auto elapsedSecondsLmuGuard
        = measureAverageActionTime(
            increaseWithLoadModifyUpdateLevelGuardedMutex,
            workerCount,
            increasePerWorker,
            iterationCount
        );

    cout << "Avg. time (w/ lmu guard): " << elapsedSecondsLmuGuard << " seconds." << endl;
}