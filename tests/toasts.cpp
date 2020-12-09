#define FMT_HEADER_ONLY

#include "fmt/format.h"
#include "itertools/range_container.hpp"

#include <chrono>
#include <deque>
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

    auto vv1 = itertools::zip(v1, v2);

    for (auto&& [i, j] : vv1) {
        std::cout << "hi" << std::endl;
        i = 1000;
    }

    for (auto i : itertools::concat(v1, v2)) {
        std::cout << "loco" << std::endl;
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