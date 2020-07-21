#include <atomic>
#include <thread>
#include <iostream>

// Shared global variables
std::atomic<int> x{0}, y{0}, z{0};

std::atomic<bool> go{false};

unsigned const loop_count = 10;

struct values_read
{
    int x, y, z;
};

values_read values1[loop_count];
values_read values2[loop_count];
values_read values3[loop_count];
values_read values4[loop_count];
values_read values5[loop_count];

void increment(
    std::atomic<int> * var_to_increment,
    values_read *      values
)
{
    while (!go)
    {
        std::this_thread::yield();
    }

    for (unsigned i = 0; i < loop_count; ++i)
    {
        values[i].x = x.load(std::memory_order_relaxed);
        values[i].y = y.load(std::memory_order_relaxed);
        values[i].z = z.load(std::memory_order_relaxed);

        var_to_increment->store(i+1, std::memory_order_relaxed);
        std::this_thread::yield();
    }
}

void read_values(values_read * values)
{
    while (!go)
    {
        std::this_thread::yield();
    }

    for (unsigned i = 0; i < loop_count; ++i)
    {
        values[i].x = x.load(std::memory_order_relaxed);
        values[i].y = y.load(std::memory_order_relaxed);
        values[i].z = z.load(std::memory_order_relaxed);

        std::this_thread::yield();
    }
}

void print(values_read * v)
{
    for (unsigned i = 0; i < loop_count; ++i)
    {
        if (i)
        {
            std::cout << ",";
        }

        std::cout << " (" << v[i].x << ", " << v[i].y << ", " << v[i].z << ") ";
    }

    std::cout << std::endl;
}

int main()
{
    std::thread t1(increment, &x, values1);
    std::thread t2(increment, &y, values2);
    std::thread t3(increment, &z, values3);
    std::thread t4(read_values, values4);
    std::thread t5(read_values, values5);

    go = true;

    t5.join();
    t4.join();
    t3.join();
    t2.join();
    t1.join();

    print(values1);
    print(values2);
    print(values3);
    print(values4);
    print(values5);    
}