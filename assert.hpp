#pragma once

#include <boost/predef.h>

#include <atomic>
#include <exception>

#if defined(BOOST_COMP_MSVC_AVAILABLE)
#   define BK_ASSERT_BREAK __debugbreak()
#   define BK_ASSERT_LIKELY(x) !!(x)
#elif defined(BOOST_COMP_GNUC_AVAILABLE) || defined(BOOST_COMP_CLANG_AVAILABLE)
#   define BK_ASSERT_BREAK (void)0
#   define BK_ASSERT_LIKELY(x) __builtin_expect(!!(x), 1)
#endif

#define BK_ASSERT_ASSERT(EXP) \
(void)(BK_ASSERT_LIKELY(EXP) || (BK_ASSERT_BREAK, ::bkassert::state::invoke_handler(#EXP, __FILE__, __func__, __LINE__), 0))

#define BK_ASSERT_SAFE(EXP) BK_ASSERT_ASSERT(EXP)
#define BK_ASSERT(EXP) BK_ASSERT_ASSERT(EXP)
#define BK_ASSERT_OPT(EXP) BK_ASSERT_ASSERT(EXP)

#define BK_ASSERT_IMPL_CAT2(x, y) x ## y
#define BK_ASSERT_IMPL_CAT(x, y) BK_ASSERT_IMPL_CAT2(x, y)
#define BK_ASSERT_IMPL_ID BK_ASSERT_IMPL_CAT(bk_assert_guard_id_, __LINE__)

#define BK_ASSERT_GUARD(handler) ::bkassert::assert_handler_guard BK_ASSERT_IMPL_ID {(handler)}

namespace bkassert {

struct assertion_failure : public std::exception {
    virtual ~assertion_failure();

    assertion_failure() = default;
    assertion_failure(assertion_failure const&) = default;
    assertion_failure(assertion_failure&&) = default;

    assertion_failure& operator=(assertion_failure const&) = default;
    assertion_failure& operator=(assertion_failure&&) = default;

    assertion_failure(
        char const* const expr
      , char const* const file_name
      , char const* const func
      , int         const line_number
    ) noexcept
      : expression {expr}
      , file {file_name}
      , function {func}
      , line {line_number}
    {
    }

    char const* what() const noexcept override {
        return "Assertion failure.";
    }

    char const* const expression = nullptr;
    char const* const file       = nullptr;
    char const* const function   = nullptr;
    int         const line      = 0;
};

[[noreturn]] void fail_abort(char const* expr, char const* file, char const* func, int line) noexcept;
[[noreturn]] void fail_sleep(char const* expr, char const* file, char const* func, int line) noexcept;
[[noreturn]] void fail_throw(char const* expr, char const* file, char const* func, int line);

class state {
public:
    using handler_t = void (char const*, char const*, char const*, int);

    static handler_t* set_handler(handler_t* handler) noexcept;
    static handler_t* get_handler() noexcept;

    static void invoke_handler(char const* expr, char const* file, char const* func, int line);
private:
    static std::atomic<handler_t*> handler_;
};

struct assert_handler_guard {
    assert_handler_guard(assert_handler_guard const&) = delete;
    assert_handler_guard& operator=(assert_handler_guard const&) = delete;
    assert_handler_guard(assert_handler_guard&&) = delete;
    assert_handler_guard& operator=(assert_handler_guard&&) = delete;

    explicit assert_handler_guard(state::handler_t* const handler = &::bkassert::fail_throw) noexcept
        : previous_ {state::set_handler(handler)}
    {
    }

    ~assert_handler_guard() {
        state::set_handler(previous_);
    }

private:
    state::handler_t* previous_ = nullptr;
};

} //namespace bkassert
