#include <cmath>
#include <cstdio>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <random>
#include <tbb/tbb.h>

// tbb.h includes windows.h
#ifdef max
    #undef max
    #undef min
#endif // max
