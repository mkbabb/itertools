#define FMT_HEADER_ONLY

#include "../src/math.hpp"
#include "../src/range_container.hpp"
#include "fmt/format.h"

#include <chrono>
#include <deque>
#include <experimental/coroutine>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <string>
#include <vector>

int
main()
{

    std::vector<int> v1 = {1, 5, 9, 2, 8, 77};
    std::vector<int> v2 = {99, 88, 77};

    auto vv1 = itertools::concat(v1, v2, v2);

    for (auto i : vv1) {
        std::cout << "hi"
                  << "\n";
    }

    // auto zipped = itertools::zip(vv1, v2);

    // for (auto z : itertools::zip(v1, v2)) {
    //     std::cout << "hi"
    //               << "\n";
    // }

    // auto r1 = itertools::range_base();

    fmt::print("tests complete\n");
    return 0;
}