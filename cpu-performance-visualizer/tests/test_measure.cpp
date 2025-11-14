#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// ==================== TESTS ======================
extern "C" {
uint64_t measure_access_time(int *data, size_t N, int mode);
}

TEST_CASE(

    "measure_access_time increases all elements exactly once in row mode"
) {
    size_t N = 4;
    int matrix[16] = {0};

    measure_access_time(matrix, N, 0);

    for (int i = 0; i < 16; i++) {
        CHECK(matrix[i] == 1);
    }
}

TEST_CASE(


    "measure_access_time increases all elements exactly once in column mode"
) {
    size_t N = 4;
    int matrix[16] = {0};

    measure_access_time(matrix, N, 1);

    for (int i = 0; i < 16; i++) {
        CHECK(matrix[i] == 1);
    }
}

TEST_CASE(


    "measure_access_time does not crash with N=1"
) {
    int x = 0;
    CHECK_NOTHROW(measure_access_time(&x, 1, 0));
    CHECK_NOTHROW(measure_access_time(&x, 1, 1));
}
