#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <random>
#include <producer_consumer.h>

std::stringstream ss("");

TEST_CASE("No delay") {
    int i, j, res, randn;
    for (i = 0; i < 100; i++) {
        res = 0;
        for (j = 0; j < 50; j++) {
            randn = rand()%50;
            res += randn;
            ss << randn << " ";
        }
        ss << "0" << std::endl;
        CHECK(run_threads(rand()%20 + 1, 0, false, &ss) == res);
    }
}

TEST_CASE("Delay 10ms") {
    int i, j, res, randn;
    for (i = 0; i < 100; i++) {
        res = 0;
        for (j = 0; j < 50; j++) {
            randn = rand()%50;
            res += randn;
            ss << randn << " ";
        }
        ss << "0" << std::endl;
        CHECK(run_threads(rand()%20 + 1, 10, false, &ss) == res);
    }
}

TEST_CASE("Delay 1000ms, more threads") {
    int i, j, res, randn;
    for (i = 0; i < 100; i++) {
        res = 0;
        for (j = 0; j < 50; j++) {
            randn = rand()%50;
            res += randn;
            ss << randn << " ";
        }
        ss << "0" << std::endl;
        CHECK(run_threads(rand()%50 + 1, 1000, false, &ss) == res);
    }
}
