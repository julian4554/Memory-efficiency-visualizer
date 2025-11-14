
#include "doctest.h"
#include <vector>
#include <string>
#include "core/measure_runner.h"
// ==================== TESTS ======================

TEST_CASE(

    "run_measurements returns exactly one result"
) {
    auto results = run_measurements(4, nullptr);

    // Clangd-safe workaround: explizite Zwischenspeicherung
    size_t count = results.size();

    CHECK(count == 1);
}

TEST_CASE(

    "run_measurements calls callback at least once"
) {
    int count = 0;

    auto cb = [&](const std::string &) {
        count++;
    };

    run_measurements(4, cb);

    CHECK(count > 0);
}

