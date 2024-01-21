#include "my_rand.h"

#include <cstddef>
#include <random>

size_t my_rand(size_t min, size_t max)
{
    static std::mt19937 rnd(std::time(nullptr));
    return std::uniform_int_distribution<>(min, max)(rnd);
}