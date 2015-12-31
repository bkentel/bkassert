#define CATCH_CONFIG_MAIN

#include "assert.hpp"

#include <Catch/catch.hpp>

#include <string>

using namespace std::string_literals;

TEST_CASE("failures", "test") {
    BK_ASSERT_GUARD(bkassert::fail_throw);

    try {
        try {
            BK_ASSERT(false);
        } catch (std::exception const& e) {
            REQUIRE(e.what() == "Assertion failure."s);
            throw;
        }
    } catch (bkassert::assertion_failure const& e) {
        REQUIRE(e.what() == "Assertion failure."s);
        REQUIRE(e.expression == "false"s);
        REQUIRE(std::string {e.file}.rfind("test.cpp") != std::string::npos);
        REQUIRE(e.function != nullptr);
        REQUIRE(strlen(e.function) > 0);
        REQUIRE(e.line > 0);
    }
}
