#include <atomic>
#include <thread>
#include <assert.h>

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x_then_y()
{
    x.store(true, std::memory_order_relaxed);
    y.store(true, std::memory_order_relaxed);
}

void read_y_then_x()
{
    while (!y.load(std::memory_order_relaxed));

    // x.load() can return false, even if y.load() returns true, i.e. x.store() must have already happened.
    // Imagine tw_x_y was scheduled on cpu_0 when it finished x.store(). Next time tw_x_y was scheduled
    // on cpu_1 and it finished y.store(). Now tr_y_x has been scheduled on cpu_1, it got y.load() == true
    // from cache, but x.load() == false.
    // 
    if (x.load(std::memory_order_relaxed))
    {
        // atomic increment
        ++z;
    }
}

int main()
{
    x = false;
    y = false;
    z = 0;

    std::thread tw_x_y(write_x_then_y);
    std::thread tr_y_x(read_y_then_x);

    tw_x_y.join();
    tr_y_x.join();

    // assert can happen now.
    assert(z.load() != 0);
}