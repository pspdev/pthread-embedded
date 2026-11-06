// Compile with psp-g++ -std=c++11 -c tests/mutex-cxx-destruction.cpp
// after installing the pthread header and building libstdc++ against it.
// C++ mutex wrappers must release their allocated native resources.
#include <mutex>
#include <type_traits>

static_assert(!std::is_trivially_destructible<std::mutex>::value,
              "mutex must destroy its native resources");
static_assert(!std::is_trivially_destructible<std::timed_mutex>::value,
              "timed_mutex must destroy its native resources");
static_assert(!std::is_trivially_destructible<std::recursive_mutex>::value,
              "recursive_mutex must destroy its native resources");
static_assert(!std::is_trivially_destructible<std::recursive_timed_mutex>::value,
              "recursive_timed_mutex must destroy its native resources");
