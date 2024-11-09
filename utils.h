#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <random>

#include <QList>

namespace RandomNumber
{
    /*static*/ std::mt19937 &random_generator();
    template<typename _RAIter>
        void random_shuffle(_RAIter itBegin, _RAIter itEnd) { std::shuffle(itBegin, itEnd, random_generator()); }
    int random_int(int range);
    template<typename T>
        const T &random_element(const QList<T> &list) { return list.at(random_int(list.count() - 1)); }
};

#endif // UTILS_H
