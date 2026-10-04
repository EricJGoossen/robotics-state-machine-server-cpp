#include <iostream>

#include "testing/TestRunner.hpp"

int main() {
    int passed = 0;
    int failed = 0;
    for (const auto& test : testing::registry()) {
        try {
            test.fn();
            passed++;
        } catch (const testing::AssertionFailure& failure) {
            failed++;
            std::cerr << "[FAIL] " << test.suite << "." << test.name << ": " << failure.message << std::endl;
        } catch (const std::exception& e) {
            failed++;
            std::cerr << "[FAIL] " << test.suite << "." << test.name
                       << ": unexpected exception: " << e.what() << std::endl;
        } catch (...) {
            failed++;
            std::cerr << "[FAIL] " << test.suite << "." << test.name
                       << ": unexpected non-standard exception" << std::endl;
        }
    }
    std::cout << passed << " passed, " << failed << " failed, " << (passed + failed) << " total"
               << std::endl;
    return failed == 0 ? 0 : 1;
}
