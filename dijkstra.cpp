#include <array>
#include <mutex>
#include <semaphore>
#include <thread>
#include <random>
#include <iostream>

constexpr size_t N = 5;

enum class State
{
    THINKING = 0,
    HUNGRY,
    EATING
};

size_t left(size_t i)
{
    return (i - 1 + N) % N;
}

size_t right(size_t i)
{
    return (i + 1) % N;
}

std::array<State, N> state;

std::mutex critical_reg_mtx;
std::mutex output_mtx;

std::array<std::binary_semaphore, N> both_forks_available
{
    std::binary_semaphore{0}, std::binary_semaphore{0},
    std::binary_semaphore{0}, std::binary_semaphore{0},
    std::binary_semaphore{0}
};

size_t my_rand(size_t min, size_t max)
{
    static std::mt19937 rnd(std::time(nullptr));
    return std::uniform_int_distribution<>(min, max)(rnd);
}

void test(size_t i)
{
    if (state[i] == State::HUNGRY &&
        state[left(i)] != State::EATING &&
        state[right(i)] != State::EATING)
    {
        state[i] = State::EATING;
        both_forks_available[i].release();
    }
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
        state[i] = State::HUNGRY;
        {
            std::lock_guard<std::mutex> lk(output_mtx); // critical section for uninterrupted print
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
        std::lock_guard<std::mutex> lk(output_mtx); // critical section for uninterrupted print
        std::cout << "\t\t\t\t" << i << " is eating " << duration << "ms\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
}

void put_forks(size_t i)
{
    std::lock_guard<std::mutex> lk(critical_reg_mtx);
    state[i] = State::THINKING;
    test(left(i));
    test(right(i));
}

void philosopher(size_t i)
{
    while (true)
    {
        think(i);
        take_forks(i);
        eat(i);
        put_forks(i);
    }
}

int main()
{
    std::vector<std::thread> threads;
    for ( size_t i = 0; i < N; ++i )
    {
        threads.emplace_back([i]{ std::cout << i; philosopher(i);});
    }
    for (auto& t : threads)
    {
        t.join();
    }
}