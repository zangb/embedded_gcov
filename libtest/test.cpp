#include <cstdint>

#include "test/test.hpp"

uint64_t test::add_or_mult(const uint64_t x, const uint64_t y, const bool use_mult) {
    if(use_mult) {
        return x * y;
    } else {
        return x + y;
    }
}
