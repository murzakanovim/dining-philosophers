#include "my_rand.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <semaphore>

constexpr int PHILOSOPHERS_COUNT = 5;

std::vector<std::mutex> forks(PHILOSOPHERS_COUNT); // создаем вилки как мьютексы
std::counting_semaphore<4> waiter(1); // "официант"
std::mutex output_mtx;

void eat(int philosopher_number)
{
    size_t duration = my_rand(400, 800);
    {
        std::lock_guard<std::mutex> lk(output_mtx);
        std::cout << "Philosopher " << philosopher_number << " is eating " << duration << " ms\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(duration)); // философ ест
}

void think(int philosopher_number)
{
    size_t duration = my_rand(400, 800);
    {
        std::lock_guard<std::mutex> lk(output_mtx);
        std::cout << "Philosopher " << philosopher_number << " is thinking " << duration << " ms\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void put_forks(int philosopher_number)
{
    forks[philosopher_number].unlock();
    forks[(philosopher_number + 1) % PHILOSOPHERS_COUNT].unlock();
    waiter.release();
}

void take_forks(int philosopher_number)
{
    waiter.acquire();
    forks[philosopher_number].lock();
    forks[(philosopher_number + 1) % PHILOSOPHERS_COUNT].lock();
}

void philosopher(int philosopher_number)
{
    for(;;)
    {
        take_forks(philosopher_number);
        eat(philosopher_number);
        put_forks(philosopher_number);
        think(philosopher_number);
    }
}

int main()
{
    std::vector<std::thread> philosophers(PHILOSOPHERS_COUNT);
    for(size_t i = 0; i < PHILOSOPHERS_COUNT; i++)
    {
        philosophers[i] = std::thread(philosopher, i);
    }
    for(auto &p: philosophers)
    {
        p.join();
    }
}