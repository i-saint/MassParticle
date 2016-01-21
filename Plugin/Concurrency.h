#ifndef Concurrency_h
#define Concurrency_h

#ifdef WithTBB
#include <tbb/tbb.h>
#include <tbb/combinable.h>

namespace ist {

template<class IndexType, class Body>
inline void parallel_for(IndexType first, IndexType last, const Body& body)
{
    tbb::parallel_for(first, last, body);
}

template<class IndexType, class IntType, class Body>
inline void parallel_for(IndexType first, IndexType last, IntType granularity, const Body& body)
{
    typedef tbb::blocked_range<IndexType> range_t;
    tbb::parallel_for(range_t(first, last, granularity),
        [&](const range_t &r) {
            for (int i = r.begin(); i < r.end(); ++i) { body(i); }
        });
}

template<class IndexType, class IntType, class Body>
inline void parallel_for_blocked(IndexType first, IndexType last, IntType granularity, const Body& body)
{
    typedef tbb::blocked_range<IndexType> range_t;
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
template<class B0, class B1, class B2>
inline void parallel_invoke(const B0& b0, const B1& b1, const B2& b2)
{
    tbb::parallel_invoke(b0, b1, b2);
}
template<class B0, class B1, class B2, class B3>
inline void parallel_invoke(const B0& b0, const B1& b1, const B2& b2, const B3& b3)
{
    tbb::parallel_invoke(b0, b1, b2, b3);
}

using tbb::task_group;
using tbb::combinable;

} // namespace ist

#else
#include <vector>
#include <mutex>
#include <ppl.h>
#include <windows.h>

namespace ist {

template<class IndexType, class Body>
inline void parallel_for(IndexType first, IndexType last, const Body& body)
{
    concurrency::parallel_for(first, last, body);
}

template<class IndexType, class IntType, class Body>
inline void parallel_for(IndexType first, IndexType last, IntType granularity, const Body& body)
{
    concurrency::parallel_for(first, last, granularity, [&](int i) {
        IndexType beg = i;
        IndexType end = std::min<IndexType>(beg + granularity, last);
        for (int i = beg; i < end; ++i) { body(i); }
    });
}

template<class IndexType, class IntType, class Body>
inline void parallel_for_blocked(IndexType first, IndexType last, IntType granularity, const Body& body)
{
    concurrency::parallel_for(first, last, granularity, [&](int i) {
        IndexType beg = i;
        IndexType end = std::min<IndexType>(beg + granularity, last);
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
template<class B0, class B1, class B2>
inline void parallel_invoke(const B0& b0, const B1& b1, const B2& b2)
{
    concurrency::parallel_invoke(b0, b1, b2);
}
template<class B0, class B1, class B2, class B3>
inline void parallel_invoke(const B0& b0, const B1& b1, const B2& b2, const B3& b3)
{
    concurrency::parallel_invoke(b0, b1, b2, b3);
}


class task_group
{
public:
    template<class Body>
    void run(const Body &b)
    {
        m_group.run(b);
    }

    void wait()
    {
        m_group.wait();
    }

private:
    concurrency::task_group m_group;
};


class tls
{
public:
#if _WIN32
    typedef DWORD tls_key_t;
#if WindowsStoreApp
    tls() { m_key = ::FlsAlloc(nullptr); }
    ~tls() { ::FlsFree(m_key); }
    void set_value(void *value) { ::FlsSetValue(m_key, value); }
    void* get_value() { return (void *)::FlsGetValue(m_key); }
#else
    tls() { m_key = ::TlsAlloc(); }
    ~tls() { ::TlsFree(m_key); }
    void set_value(void *value) { ::TlsSetValue(m_key, value); }
    void* get_value() const { return (void *)::TlsGetValue(m_key); }
#endif
#else
    typedef pthread_key_t tls_key_t;
    tls() { pthread_key_create(&m_key, nullptr); }
    ~tls() { pthread_key_delete(m_key); }
    void set_value(void *value) { pthread_setspecific(m_key, value); }
    void* get_value() const { return pthread_getspecific(m_key); }
#endif

private:
    tls_key_t m_key;
};

template<class T>
class combinable
{
public:
    combinable()
    {
    }

    ~combinable()
    {
        std::unique_lock<std::mutex> lock;
        for (auto p : m_locals) { delete p; }
        m_locals.clear();
    }

    T& local()
    {
        void *value = m_tls.get_value();
        if (value == nullptr) {
            T *v = new T();
            m_tls.set_value(v);
            value = v;

            {
                std::unique_lock<std::mutex> lock;
                m_locals.push_back(v);
            }
        }
        return *(T*)value;
    }

    template<class Body>
    void combine_each(const Body& body)
    {
        for (auto p : m_locals) { body(*p); }
    }

private:
    tls m_tls;
    std::mutex m_mutex;
    std::vector<T*> m_locals;
};

} // namespace ist

#endif // WithTBB

#endif // Concurrency_h
