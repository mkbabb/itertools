#define FMT_HEADER_ONLY

#include <fmt/format.h>
#include <iostream>
#include <itertools/itertools.hpp>
#include <list>
#include <string>
#include <vector>

int
main()
{
    using namespace itertools::views;

    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7};
    std::list<std::string> l = {"a", "b", "c", "a"};

    auto pred = [](auto&& tup) { return std::get<1>(tup) == "a"; };

    auto func = [](auto&& tup) {
        std::get<0>(tup) += 100;
        std::get<1>(tup) += "bc";
        return tup;
    };

    auto pipeline =
        zip(v, l, std::vector<int>{99, 88, 77, 66}) | filter(pred) | transform(func);

    for (auto [n, tup] : enumerate(pipeline)) {
        auto [i, j, k] = tup;
        std::cout << fmt::format("{}: {}, {}, {}\n", n, i, j, k);
    }
}