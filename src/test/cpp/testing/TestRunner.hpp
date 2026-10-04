#pragma once

// A minimal, dependency-free test harness standing in for JUnit 5 on the
// C++ side. A TEST(Suite, Name) block registers itself at static-init time;
// RunTests.cpp's main() runs every registered test and reports a summary.
//
// Assertion macros throw testing::AssertionFailure on failure, which aborts
// just the current test (mirroring JUnit's fail-fast assertXxx behavior)
// without aborting the whole run.

#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace testing {

struct TestCase {
    std::string suite;
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(std::string suite, std::string name, std::function<void()> fn) {
        registry().push_back(TestCase{std::move(suite), std::move(name), std::move(fn)});
    }
};

struct AssertionFailure {
    std::string message;
};

} // namespace testing

#define TEST(suite, name)                                                                    \
    static void suite##_##name##_testfn();                                                   \
    static ::testing::Registrar suite##_##name##_registrar(#suite, #name, suite##_##name##_testfn); \
    static void suite##_##name##_testfn()

#define TG_FAIL(msg)                                                                    \
    do {                                                                                \
        std::ostringstream tg_oss;                                                      \
        tg_oss << msg << " (at " << __FILE__ << ":" << __LINE__ << ")";                 \
        throw ::testing::AssertionFailure{tg_oss.str()};                                \
    } while (0)

#define EXPECT_TRUE(cond)                                          \
    do {                                                           \
        if (!(cond)) {                                             \
            TG_FAIL("EXPECT_TRUE failed: " << #cond);              \
        }                                                          \
    } while (0)

#define EXPECT_FALSE(cond)                                         \
    do {                                                           \
        if (cond) {                                                \
            TG_FAIL("EXPECT_FALSE failed: " << #cond);             \
        }                                                          \
    } while (0)

#define EXPECT_EQ(a, b)                                                         \
    do {                                                                        \
        if (!((a) == (b))) {                                                    \
            TG_FAIL("EXPECT_EQ failed: " << #a << " == " << #b);                \
        }                                                                       \
    } while (0)

#define EXPECT_NE(a, b)                                                         \
    do {                                                                        \
        if ((a) == (b)) {                                                       \
            TG_FAIL("EXPECT_NE failed: " << #a << " != " << #b);                \
        }                                                                       \
    } while (0)

// Expects `stmt` to throw some std::exception (matching Java's
// assertThrows(IllegalArgumentException.class, ...); this harness does not
// distinguish exception types the way JUnit does).
#define EXPECT_THROWS(stmt)                                                     \
    do {                                                                        \
        bool tg_threw = false;                                                  \
        try {                                                                   \
            stmt;                                                               \
        } catch (const std::exception&) {                                       \
            tg_threw = true;                                                    \
        }                                                                       \
        if (!tg_threw) {                                                        \
            TG_FAIL("EXPECT_THROWS: expected an exception from: " << #stmt);    \
        }                                                                       \
    } while (0)

// Expects `stmt` to complete without throwing (used where a Java test's
// only assertion is "no exception is the assertion").
#define EXPECT_NO_THROW(stmt)                                                              \
    do {                                                                                   \
        try {                                                                              \
            stmt;                                                                          \
        } catch (const std::exception& tg_e) {                                             \
            TG_FAIL("EXPECT_NO_THROW: unexpected exception from " << #stmt << ": " << tg_e.what()); \
        }                                                                                   \
    } while (0)
