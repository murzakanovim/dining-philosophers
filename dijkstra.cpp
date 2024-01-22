#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <semaphore>

#include "my_rand.h"

enum class State
{
    THINKING = 0,
    HUNGRY,
    EATING
};


std::array<std::binary_semaphore, 5> both_forks_available
{
    std::binary_semaphore{0}, std::binary_semaphore{0},
    std::binary_semaphore{0}, std::binary_semaphore{0},
    std::binary_semaphore{0}
};

class DinnerTable
{
    std::vector<State> states;
    std::vector<int> eat_counts;

    std::mutex critical_reg_mtx;
    std::mutex output_mtx;

    size_t left(size_t i)
    {
        return (i + states.size() - 1) % states.size();
    }

    size_t right(size_t i)
    {
        return (i + 1) % states.size();
    }

    void test(size_t i)
    {
        if (states[i] == State::HUNGRY &&
            states[left(i)] != State::EATING &&
            states[right(i)] != State::EATING)
        {
            states[i] = State::EATING;
            both_forks_available[i].release();
        }
    }

public:
    explicit DinnerTable(size_t N)
        :
        states(N, State::THINKING),
        eat_counts(N, 0)
        {
        }

    void think(size_t i)
    {
        size_t duration = my_rand(400, 800);
        {
            std::lock_guard<std::mutex> lk(output_mtx);
            std::cout << i << " is thinking " << duration << "ms\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }

    void take_forks(size_t i)
    {
        {
            std::lock_guard<std::mutex> lk(critical_reg_mtx);
            states[i] = State::HUNGRY;
            {
                std::lock_guard<std::mutex> lk(output_mtx);
                std::cout << "\t\t" << i << " is State::HUNGRY\n";
            }
            test(i);
        }
        both_forks_available[i].acquire();
    }

    void eat(size_t i)
    {
        size_t duration = my_rand(400, 800);
        {
            std::lock_guard<std::mutex> lk(output_mtx);
            std::cout << "\t\t\t\t" << i << " is eating " << duration << "ms\n";
        }
        eat_counts[i]++;  // Increase eat count for philosopher "i"
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }

    void put_forks(size_t i)
    {
        std::lock_guard<std::mutex> lk(critical_reg_mtx);
        states[i] = State::THINKING;
        test(left(i));
        test(right(i));
    }

    bool continue_eating(size_t i)
    {
        return eat_counts[i] < 10;
    }
};

void philosopher(size_t i, DinnerTable& table)
{
    while (table.continue_eating(i))
    {
        table.think(i);
        table.take_forks(i);
        table.eat(i);
        table.put_forks(i);
    }
}

int main()
{
    size_t N = 5;
    std::vector<std::thread> threads;
    DinnerTable table(N);

    for (size_t i = 0; i < N; ++i)
    {
        threads.emplace_back(philosopher, i, std::ref(table));
    }
    for (auto&& t : threads)
    {
        t.join();
    }
    return 0;
}