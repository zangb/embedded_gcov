#include "test.hpp"
extern "C" {
#include <gcov/gcov.h>
}

int main() {
    // test::add_or_mult(3, 3, false);
    test::add_or_mult(2, 2, true);
    return 0;
}
