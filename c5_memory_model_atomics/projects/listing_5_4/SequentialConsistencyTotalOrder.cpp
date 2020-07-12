#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void WriteX()
{
    // Sequentially consistent: all threads see the same order of operations on atomics
    x.store(true, std::memory_order_seq_cst);
}

void WriteY()
{
    y.store(true, std::memory_order_seq_cst);
}

// When ReadXThenY completes, it is guaranteed that WriteX has happened.
// Therefore, WriteX 'happens-before' ReadXThenY
// So the order is WriteX < ReadXThenY
//
void ReadXThenY()
{
    // Until WriteX happens, keep spinning
    while (!x.load(std::memory_order_seq_cst));

    // Now x == true, if WriteY has happened then increase z
    // if WriteY has not happened yet, z remains the same
    //
    if (y.load(std::memory_order_seq_cst))
    {
        ++z;
    }
}

// When ReadYThenX completes, it is guranteed that WriteY has happened.
// Therefore, WriteY 'happens-before' ReadYThenX
// So the order is WriteY < ReadYThenX
//
void ReadYThenX()
{
    // Until WriteY happens, keep spinning
    while (!y.load(std::memory_order_seq_cst));

    // Now y == true, if WriteX has happened then increase z
    // if WriteX has not happened yet, z remains the same
    //
    if (x.load(std::memory_order_seq_cst))
    {
        ++z;
    }
}

int main()
{
    x = false;
    y = false;
    z = 0;

    std::thread twx(WriteX);
    std::thread twy(WriteY);
    std::thread trxy(ReadXThenY);
    std::thread tryx(ReadYThenX);

    twx.join();
    twy.join();
    trxy.join();
    tryx.join();

    // z.load() == 0 could happen only if      ReadXThenY finished before WriteY 
    //                                    and  ReadYThenX finished before WriteX.
    //
    // Note, the above is correct only if we are using sequentially consistent memory order.
    // Because, with only sequentially consistent order, a store happens-before a load.
    //
    // Since WriteY < ReadYThenX, ReadXThenY must have finished before ReadYThenX.
    // Since WriteX < ReadXThenY, ReadYThenX must have finished before ReadXThenY.
    // We have a contradiction here. Therefore, with sequential consistency, z.load() != 0 must be true.
    //
    assert( z.load() != 0 );
}