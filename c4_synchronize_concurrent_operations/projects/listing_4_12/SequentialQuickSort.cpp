#include <iostream>
#include <list>
#include <random>

using namespace std;

template<typename T>
list<T> SequentialQuickSort(list<T> input)
{
    if (input.empty())
    {
        return input;
    }

    list<T> result;

    // extract element at input.begin() and insert at result.begin()
    result.splice(result.begin(), input, input.begin());

    // take first element of input as the pivot
    T const & pivot = *result.begin();

    // partition remaining input elements around pivot
    // divisionPoint is the first element in input going rightwards that is bigger than pivot.
    auto divisionPoint = 
        partition(
            input.begin(),
            input.end(),
            [&] (T const & t)
            {
                return t < pivot;
            });

    list<T> lowerPart;

    // lowerPart holds elements < pivot, input will contain elements >= pivot, except pivot itself
    lowerPart.splice(lowerPart.end(), input, input.begin(), divisionPoint);

    // sort lowerPart recursively
    auto sortedLower = SequentialQuickSort(move(lowerPart));

    // sort >= pivot (except pivot) elements recursively
    auto sortedHigher = SequentialQuickSort(move(input));

    // the order of the below two splice calls matter
    //
    // result = <pivot, sortedHigher>
    result.splice(result.end(), sortedHigher);

    // result = <sortedLower, pivot, sortedHigher>
    result.splice(result.begin(), sortedLower);

    return result;
}

void PrintList(list<int> const & elements)
{
    for (int x : elements)
    {
        cout << x << " ";
    }

    cout << endl;
}

int main()
{
    const int kMaxElmenetCount = 10;
    random_device device;
    mt19937 rng(device());
    uniform_int_distribution<mt19937::result_type> distMax(1, kMaxElmenetCount);

    list<int> elements;
    for (int i = 0; i < kMaxElmenetCount; ++i)
    {
        elements.push_back( distMax(rng) );
    }

    PrintList(elements);

    elements = SequentialQuickSort<int>(ref(elements));

    PrintList(elements);
}