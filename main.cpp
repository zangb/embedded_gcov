extern "C" {
#include <gcov/gcov.h>
}

#include <test/test.hpp>

int main() {
    test::add_or_mult(3, 3, false);
    return 0;
}
