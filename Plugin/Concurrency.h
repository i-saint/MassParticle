#ifndef Concurrency_h
#define Concurrency_h

#define WithTBB

#ifdef WithTBB
#include <tbb/tbb.h>

template<class IntType, class Body>
inline void parallel_for(IntType first, IntType last, const Body& body)
{
    tbb::parallel_for(first, last, body);
}

template<class IntType, class Body>
inline void parallel_for(IntType first, IntType last, IntType granularity, const Body& body)
{
    typedef tbb::blocked_range<IntType> range_t;
    tbb::parallel_for(range_t(first, last, granularity),
        [&](const range_t &r) {
            for (int i = r.begin(); i < r.end(); ++i) { body(i); }
        });
}

template<class IntType, class Body>
inline void parallel_for_blocked(IntType first, IntType last, IntType granularity, const Body& body)
{
    typedef tbb::blocked_range<IntType> range_t;
    tbb::parallel_for(range_t(first, last, granularity), [&](const range_t &r) { body(r.begin(), r.end()); });
}

template<class Iter, class Compare>
inline void parallel_sort(Iter beg, Iter end, const Compare& compare)
{
    tbb::parallel_sort(beg, end, compare);
}

template<class B0, class B1>
inline void parallel_invoke(const B0& b0, const B1& b1)
{
    tbb::parallel_invoke(b0, b1);
}

#else
#include <ppl.h>

template<class IntType, class Body>
inline void parallel_for(IntType first, IntType last, const Body& body)
{
    concurrency::parallel_for(first, last, body);
}

template<class IntType, class Body>
inline void parallel_for(IntType first, IntType last, int granularity, const Body& body)
{
    concurrency::parallel_for(first, last, granularity, [&](int i) {
        IntType beg = i;
        IntType end = std::min<IntType>(beg + granularity, last);
        for (int i = beg; i < end; ++i) { body(i); }
    });
}

template<class IntType, class Body>
inline void parallel_for_blocked(IntType first, IntType last, IntType granularity, const Body& body)
{
    concurrency::parallel_for(first, last, granularity, [&](int i) {
        IntType beg = i;
        IntType end = std::min<IntType>(beg + granularity, last);
        body(beg, end);
    });
}

template<class Iter, class Compare>
inline void parallel_sort(Iter beg, Iter end, const Compare& compare)
{
    concurrency::parallel_sort(beg, end, compare);
}

template<class B0, class B1>
inline void parallel_invoke(const B0& b0, const B1& b1)
{
    concurrency::parallel_invoke(b0, b1);
}

#endif


#endif // Concurrency_h
