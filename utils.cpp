#include "utils.h"

std::mt19937 &RandomNumber::random_generator()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

int RandomNumber::random_int(int range)
{
    std::uniform_int_distribution<> distr(0, range);
    return distr(random_generator());
}
