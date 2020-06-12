#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

using namespace std;

using ulong = unsigned long;

template<typename Iterator, typename T>
struct BlockAccumulator
{
    void operator() (Iterator first, Iterator last, T & result)
    {
        result = accumulate(first, last, result);
    }
};

template<typename Iterator, typename T>
T ParallelAccumulate(Iterator first, Iterator last, T initialValue)
{
    ulong const span = distance(first, last);

    if (span <= 0)
    {
        return initialValue;
    }

    ulong const minimumItemPerThread = 10;

    // span/minimumItemPerThread = fullBlockCount these many threads each will handle one block
    // Remaining items (if any) will be handled by master thread
    //
    ulong const maxThreadCount = (span+minimumItemPerThread-1)/minimumItemPerThread;

    ulong const hardwareThreadCount = thread::hardware_concurrency();

    ulong const absoluteMinThreadCount = 2;

    ulong const threadCount = min(max(absoluteMinThreadCount, hardwareThreadCount), maxThreadCount);

    vector<T> blockResults(threadCount);

    // We already are on a thread, so need to spawn one less additional threads
    vector<thread> additionalThreads(threadCount-1);

    ulong const blockLength = span/threadCount;

    Iterator blockStart = first;
    for (ulong i = 0; i < threadCount-1; ++i)
    {
        Iterator blockEnd = blockStart;
        advance(blockEnd, blockLength);
        additionalThreads[i] =
            thread(
                BlockAccumulator<Iterator, T>(),
                blockStart,
                blockEnd,
                ref(blockResults[i]));
        blockStart = blockEnd;
    }

    // Accumulate remaining items on current thread
    BlockAccumulator<Iterator, T>()(blockStart, last, blockResults[threadCount-1]);

    // wait for the spawned threads
    for_each(additionalThreads.begin(), additionalThreads.end(), mem_fn(&thread::join));

    // Accumulate all blocks
    return accumulate(blockResults.cbegin(), blockResults.cend(), initialValue);
}

int main()
{
    int elementCount = 300000000;
    vector<int> numbers(elementCount, 1);

    // There were 4 hardware threads.
    // Only when elementCount reached in the millions, did parallel outperform sequential.
    // For 300M elements
    //  - Sequential Elapsed: 2.22675 seconds.
    //  - Parallel Elapsed: 0.987583 seconds.
    //
    auto startTime = chrono::high_resolution_clock::now();
    int totalSeq = accumulate(numbers.cbegin(), numbers.cend(), 0);
    auto finishTime = chrono::high_resolution_clock::now();

    auto seqDuration = finishTime-startTime;
    cout << "Sequential Elapsed: " << seqDuration.count()/1e9 << " seconds." << endl;

    startTime = chrono::high_resolution_clock::now();
    int totalPar = ParallelAccumulate<vector<int>::const_iterator, int>(numbers.cbegin(), numbers.cend(), 0);
    finishTime = chrono::high_resolution_clock::now();

    auto parallelDuration = finishTime-startTime;
    cout << "Parallel Elapsed: " << parallelDuration.count()/1e9 << " seconds." << endl;
}