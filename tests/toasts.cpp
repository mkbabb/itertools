#define FMT_HEADER_ONLY

#include "fmt/format.h"
#include "itertools/range_iterator.hpp"
#include "itertools/views/all.hpp"

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
    using namespace itertools;

    std::vector<int> v1 = {1, 5, 9, 2, 8, 77, 88};

    auto vv1 = views::zip(v1, std::vector<int>{99, 88, 77});

    for (auto [i, j] : vv1) {
        std::cout << fmt::format("{}, {}\n", i, j);
    }

    // v1[0] = 1000;

    // int y = 1;
    // int& x = y;

    // for (auto&& i : vv1) {
    //     std::cout << "hi" << std::endl;
    //     // x = i;
    // }

    // for (auto&& [i, j] : vv1) {
    //     std::cout << "hi" << std::endl;
    // }

    std::vector<std::vector<int>> vat = {{1, 3}, {4}, {7, 8, 9}, {8, 9}};
    std::vector vat2 = {vat, vat};

    for (auto i : views::flatten(views::flatten(vat2)) | views::intersperse(9999)) {
        std::cout << i << std::endl;
    }

    // auto f = [](auto v) { return v; };

    // for (auto i : to_range(v1) | views::stride(2) | views::transform(f)) {
    //     std::cout << i << std::endl;
    // }

    // auto zipped = views::zip(vv1, v2);

    // for (auto z : views::zip(v1, v2)) {
    //     std::cout << "hi"
    //               << "\n";
    // }

    fmt::print("tests complete\n");
    return 0;
}