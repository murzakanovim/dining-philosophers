#include "my_rand.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <semaphore>

constexpr int N = 5;

std::counting_semaphore<N - 1> waiter(1);

class DiningTable {
public:
    explicit DiningTable(int N)
        : forks(N),
          eat_counts(N, 0) {}

    void eat(int philosopher_number)
    {
        size_t duration = my_rand(400, 800);
        {
            std::lock_guard<std::mutex> lk(output_mtx);
            std::cout << "Philosopher " << philosopher_number << " is eating " << duration << " ms\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(duration)); // философ ест
        eat_counts[philosopher_number]++;
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
        forks[(philosopher_number + 1) % forks.size()].unlock();
        waiter.release();
    }

    void take_forks(int philosopher_number)
    {
        waiter.acquire();
        forks[philosopher_number].lock();
        forks[(philosopher_number + 1) % forks.size()].lock();
    }

    bool should_continue(int philosopher_number)
    {
        return eat_counts[philosopher_number] < 10;
    }

private:
    std::vector<std::mutex> forks;
    std::vector<int> eat_counts;
    std::mutex output_mtx;
};

void philosopher(int philosopher_number, DiningTable& table)
{
    while (table.should_continue(philosopher_number))
    {
        table.take_forks(philosopher_number);
        table.eat(philosopher_number);
        table.put_forks(philosopher_number);
        table.think(philosopher_number);
    }
}

int main()
{
    DiningTable table(N);
    std::vector<std::thread> philosophers(N);
    for(size_t i = 0; i < N; i++)
    {
        philosophers[i] = std::thread(philosopher, i, std::ref(table));
    }

    for(auto &&p: philosophers)
    {
        p.join();
    }
}