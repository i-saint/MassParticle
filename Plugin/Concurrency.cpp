#include "pch.h"
#include "Concurrency.h"

#ifndef WithTBB

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
#endif

#if _WIN32||_WIN64
#if WindowsStoreApp
tls::tls() { m_key = ::FlsAlloc(nullptr); }
tls::~tls() { ::FlsFree(m_key); }
void tls::set_value(void * value) { ::FlsSetValue(m_key, (LPVOID)value); }
void* tls::get_value() { return (void *)::FlsGetValue(m_key); }
#else
tls::tls() { m_key = ::TlsAlloc(); }
tls::~tls() { ::TlsFree(m_key); }
void tls::set_value(void *value) { ::TlsSetValue(m_key, (LPVOID)value); }
void* tls::get_value() const { return (void *)::TlsGetValue(m_key); }
#endif
#else
tls::tls() { pthread_key_create(&m_key, nullptr); }
tls::~tls() { pthread_key_delete(m_key); }
void tls::set_value(void * value) { pthread_setspecific(m_key, value); }
void* tls::get_value() const { return pthread_getspecific(m_key); }
#endif

#endif // WithTBB
