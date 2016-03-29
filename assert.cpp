#include "assert.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#if defined(BOOST_OS_WINDOWS_AVAILABLE)
#include <crtdbg.h> // _CrtSetReportMode

extern "C" {
    __declspec(dllimport) void __stdcall Sleep(std::uint32_t dwMilliseconds);
};

#else // assume posix
#include <unistd.h> //sleep
#endif

namespace bkassert {

assertion_failure::~assertion_failure() = default;

std::atomic<state::handler_t*> state::handler_ {&::bkassert::fail_abort};

state::handler_t* state::set_handler(state::handler_t* const handler) noexcept {
    return std::atomic_exchange(&handler_, handler);
}

state::handler_t* state::get_handler() noexcept {
    return handler_.load();
}

void state::invoke_handler(
    char const* const expr
  , char const* const file
  , char const* const func
  , int         const line
) {
    handler_.load()(expr, file, func, line);
}

namespace {

#if defined(BOOST_OS_WINDOWS_AVAILABLE)
void do_sleep() {
    Sleep(1000);
}
#else // assume posix
void do_sleep() {
    sleep(1);
}
#endif

void print_error(
    char const* const expr
  , char const* const file
  , char const* const func
  , int         const line
) noexcept {
    auto const get_string = [](char const* const str) noexcept {
        if (str) {
            if (*str) {
                return str;
            }

            return "(empty)";
        }
        return "(null)";
    };

    std::fprintf(stderr
      , "Assertion failed: \"%s\", %s:%d (%s)\n"
      , get_string(expr), get_string(file), line, get_string(func));

    std::fflush(stderr);
}

} //namespace

void fail_abort(
    char const* const expr
  , char const* const file
  , char const* const func
  , int         const line
) noexcept {
#if defined(BOOST_OS_WINDOWS_AVAILABLE)
    _CrtSetReportMode(_CRT_ASSERT, 0);
    _CrtSetReportMode(_CRT_ERROR,  0);
    _CrtSetReportMode(_CRT_WARN,   0);

    _set_abort_behavior(0, _CALL_REPORTFAULT);
#endif

    print_error(expr, file, func, line);
    std::abort();
}

void fail_sleep(
    char const* const expr
  , char const* const file
  , char const* const func
  , int         const line
) noexcept {
    print_error(expr, file, func, line);

    for (;;) {
        do_sleep();
    }
}

void fail_throw(
    char const* const expr
  , char const* const file
  , char const* const func
  , int         const line
) {
    if (!std::uncaught_exception()) {
        throw assertion_failure {expr, file, func, line};
    }

    std::fprintf(stderr, "assertion_failure cannot be thrown due to pending uncaught exception.\n");
    fail_abort(expr, file, func, line);
}

} //namespace bkassert
