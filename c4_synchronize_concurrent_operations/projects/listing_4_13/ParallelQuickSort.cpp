#include <chrono>
#include <future>
#include <iostream>
#include <list>
#include <random>
#include <thread>

using namespace std;
using namespace chrono;

// Functional programming style makes code simpler, no locking involved.
template <typename T>
list<T> ParallelQuickSort(list<T> & input)
{
    if (input.empty())
    {
        return input;
    }

    list<T> result;
    result.splice(result.begin(), input, input.begin());

    T const & pivot = *result.begin();

    auto divisionPoint =
        partition(
            input.begin(),
            input.end(),
            [&] (T const & t)
            {
                return t < pivot;
            }
        );

    list<T> lowerPart;
    lowerPart.splice(lowerPart.end(), input, input.begin(), divisionPoint);

    auto sortedLowerFuture = async( &ParallelQuickSort<T>, move(lowerPart) );
    auto sortedHigher = ParallelQuickSort(input);

    result.splice(result.end(), sortedHigher);
    result.splice(result.begin(), sortedLowerFuture.get());

    return result;
}

void PrintList(list<int> elements)
{
    for (int x : elements)
    {
        cout << x << " ";
    }

    cout << endl;
}

// list.sort()   : Took 0.6769 milli seconds to sort 2000 elements.
// SequentialSort: Took 8.697 milli seconds to sort 2000 elements.
// ParallelSort  : Took 232.222 milli seconds to sort 2000 elements.
int main()
{
    const int kMaxElementCount = 2000;
    random_device device;
    mt19937 rng(device());
    uniform_int_distribution<mt19937::result_type> dist(1, kMaxElementCount);

    list<int> elements;
    for (int i = 0; i < kMaxElementCount; ++i)
    {
        elements.push_back( dist(rng) );
    }

    // PrintList(elements);

    auto const & tic = steady_clock::now();

    elements = ParallelQuickSort(elements);
    // elements.sort();

    auto const & toc = steady_clock::now();

    auto const & elapsedMilliSec = duration_cast<duration<double, ratio<1, 1000>>>(toc-tic);

    cout << "Took " << elapsedMilliSec.count() << " milli seconds to sort " << kMaxElementCount << " elements." << endl; 

    // PrintList(elements);
}