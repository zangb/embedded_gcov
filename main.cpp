#include <stdlib.h>
extern "C" {
#include <gcov/gcov.h>
}

#include <test/test.hpp>

int main() {
    unsigned char* gcov_area = new unsigned char[262144];
    gcov_unsigned_t* gcda_area = new gcov_unsigned_t[16368];
    set_gcov_buffer(gcov_area, 262144);
    set_gcov_gcda_buffer(gcda_area, 16368);
    test::add_or_mult(3, 3, false);
    return 0;
}
