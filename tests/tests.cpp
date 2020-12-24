#define FMT_HEADER_ONLY

#include "fmt/format.h"

#include <chrono>
#include <deque>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <string>
#include <vector>

#include "itertools/algorithm/all.hpp"
#include "itertools/itertools.hpp"
#include "itertools/range_iterator.hpp"
#include "itertools/views/all.hpp"

using namespace itertools;

void
zip_tests()
{
    using namespace itertools::views;
    {
        std::vector<int> iv1 = { 101, 102, 103, 104 };
        std::vector<int> iv2 = { 9, 8, 7, 6, 5, 4, 3, 2 };
        std::vector<int> iv3 = { 123, 1234, 12345 };

        for (auto&& [i, j, k] : zip(iv1, iv2, iv3)) {
        };
    }
    {
        std::vector<std::string> sv1 = { "\nThis", "but", "different", "containers," };
        std::vector<std::string> sv2 = { "is", "can", "types", "too?" };
        std::vector<std::string> sv3 = { "cool,", "we iterate through", "of", "...\n" };

        for (auto&& [i, j, k] : zip(sv1, sv2, sv3)) {
        };
    }
    {

        std::list<std::string> sl1 = {
            "Yes, we can!", "Some more numbers:", "More numbers!"
        };
        std::vector<std::string> sv1 = { "Different types, too!",
                                         "Ints and doubles.",
                                         "" };
        std::list<int> iv1 = { 1, 2, 3 };
        std::vector<double> dv1{ 3.141592653589793238, 1.6181, 2.71828 };

        for (auto&& [i, j, k, l] : zip(sl1, sv1, iv1, dv1)) {
        }
    }
    {
        std::map<int, int> id1 = { { 0, 10 }, { 1, 11 }, { 2, 12 },
                                   { 3, 13 }, { 4, 14 }, { 5, 15 } };
        std::list<std::string> sv1 = { "1", "mijn", "worten", "2", "helm", "dearth" };
        std::vector<double> dv1 = { 1.2, 3.4, 5.6, 6.7, 7.8, 8.9, 9.0 };

        for (auto&& [i, j, k, l] : zip(id1, sv1, dv1, dv1)) {
            auto [key, value] = i;
            fmt::print("{}: {}, {}, {}, {}\n", key, value, j, k, l);
        }
    }
}

void
tupletools_tests()
{
    using namespace itertools::views;
    {
        auto tup1 = std::make_tuple(1, 2, 3, 4);
        assert(tupletools::to_string(tup1) == "(1, 2, 3, 4)");
    }
    {
        auto tup1 = tupletools::make_tuple_of<20>(true);
        assert(tupletools::tuple_size<decltype(tup1)>::value == 20);
    }
    {
        std::vector<std::tuple<int, int>> tv1 = {
            { 1, 2 }, { 3, 4 }, { 5, 6 }, { 7, 8 }
        };
        for (auto&& v : enumerate(tv1)) {
            auto [n, i, j] = tupletools::flatten(v);
            fmt::print("{} {} {}\n", n, i, j);
        }
    }
    {
        auto tup1 = std::make_tuple('a', 'b', 'c', 'd');
        tupletools::roll<false>(tup1);
        assert(tup1 == std::make_tuple('d', 'a', 'b', 'c'));
    }
}

// void
// itertools_tests()
// {
//     {
//         std::vector<std::string> sv1 = { "h", "e", "l", "l", "o" };
//         std::vector<int> iv1 = { 1, 2, 3, 4, 5, 6, 7, 8 };

//         assert(itertools::join(sv1, "") == "hello");
//         assert(itertools::join(sv1, ",") == "h,e,l,l,o");
//         assert(itertools::join(iv1, ", ") == "1, 2, 3, 4, 5, 6, 7, 8");
//     }
//     {
//         std::vector<int> iv1 = { 1, 2, 3, 4, 5, 6, 7, 8 };
//         itertools::roll(itertools::roll(iv1));
//         assert(iv1 == (std::vector<int>{ 7, 8, 1, 2, 3, 4, 5, 6 }));
//     }
// }

void
any_tests()
{
    {
        // Any tests with initializer list
        auto tup1 = std::make_tuple(1, 2, 3, 4, 5, 6);
        auto tup2 = std::make_tuple(33, 44, 77, 4, 5, 99);

        auto tup_bool =
          tupletools::where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);
        bool bool1 = tupletools::any_of(tup_bool);
        bool bool2 =
          tupletools::any_where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);

        assert(bool1 == true);
        assert(bool1 = bool2);
    }
    {
        // Any tests with tuple of booleans.
        auto tup_bool1 = std::make_tuple(true, true, true, true, false);
        auto tup_bool2 = std::make_tuple(false, false, false, false, false);

        bool bool1 = tupletools::any_of(tup_bool1);
        bool bool2 = tupletools::any_of(tup_bool2);

        assert(bool1 == true);
        assert(bool2 == false);
    }
    {
        // All tests
        auto tup1 = std::make_tuple(1, 2, 3, 4, 5, 6);
        auto tup2 = std::make_tuple(1, 2, 3, 4, 5, 6);

        auto tup_bool =
          tupletools::where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);
        bool bool1 = tupletools::all_of(tup_bool);
        bool bool2 =
          tupletools::all_where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);

        assert(bool1 == true);
        assert(bool1 = bool2);
    }
    {
        // All tests with tuple of booleans.
        auto tup_bool1 = std::make_tuple(true, true, true, true, false);
        auto tup_bool2 = std::make_tuple(true, true, true, true, true);

        bool bool1 = tupletools::all_of(tup_bool1);
        bool bool2 = tupletools::all_of(tup_bool2);

        assert(bool1 == false);
        assert(bool2 == true);
    }
    {
        // Disjunction tests
        auto tup1 = std::make_tuple(1, 2, 3, 4);
        auto tup2 = std::make_tuple(1, 2, 7, 4);

        auto ilist =
          tupletools::where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);

        bool bool1 = tupletools::disjunction_of(ilist);
        assert(bool1 == false);
    }
    {
        // Disjunction tests with tuple of booleans.
        auto tup_bool1 = std::make_tuple(true, true, true, false, false);
        auto tup_bool2 = std::make_tuple(true, true, false, true, true);

        bool bool1 = tupletools::disjunction_of(tup_bool1);
        bool bool2 = tupletools::disjunction_of(tup_bool2);

        assert(bool1 == true);
        assert(bool2 == false);
    }
}

void
enumerate_tests()
{
    using namespace itertools::views;
    auto tup = std::make_tuple(1, 2);
    {
        std::vector<int> v1(100000, 0);
        int j = 0;
        int k = 0;
        for (auto [n, i] : enumerate(v1)) {
            j++;
            k = n;
            std::get<0>(tup);
        }
        assert((j - 1) == k);
    }
    {
        std::vector<int> v1(1000000, 0);
        int j = 0;
        int k = 0;

        for (auto [n, i] : enumerate(v1)) {
            j++;
            k = n;
            std::get<0>(tup);
        }
        assert((j - 1) == k);
    }
}

void
iota_tests()
{
    using namespace itertools::views;

    {
        int stop = -999'999;
        int j = stop;
        auto _iota = iota(stop);
        for (auto i : _iota) {
            assert(i == j);
            j++;
        }
        assert(j == 0);
    }
    {
        int stop = 1'999'999;
        int j = 0;
        auto _iota = iota(stop);
        for (auto i : _iota) {
            assert(i == j);
            j++;
        }
        assert(j == stop);
    }

    {
        int stop = -999'999;
        int j = stop;
        auto _iota = iota(stop, 0);
        for (auto i : _iota) {
            assert(i == j);
            j++;
        }
        assert(j == 0);
    }
}

// void
// to_string_tests(bool print = false)
// {
//     {
//         auto iter = std::make_tuple(1, 2, 3, 4, 5, 6);
//         auto ndim = itertools::detail::get_ndim(iter);
//         std::string s = itertools::to_string(iter);
//         if (print) {
//             std::cout << s << std::endl;
//         }
//     }
//     {
//         auto iter = std::make_tuple(
//             std::make_tuple(1, 2, 3, 4, 5, 6),
//             std::make_tuple(1, 2, 3, 4, 5, 6),
//             std::make_tuple(1, 2, 3, 4, 5, 6));
//         auto ndim = itertools::detail::get_ndim(iter);
//         std::string s = itertools::to_string_f(iter, [](auto&& v) -> std::string {
//             return " " + std::to_string(v) + " ";
//         });
//         if (print) {
//             std::cout << s << std::endl;
//         }
//     }
//     {
//         std::vector<std::map<int, int>> iter = {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}};
//         auto ndim = itertools::detail::get_ndim(iter);
//         std::string s = itertools::to_string(iter);
//         if (print) {
//             std::cout << s << std::endl;
//         }
//     }
//     {
//         std::vector<std::tuple<std::vector<std::vector<std::vector<int>>>, int>> iter
//         =
//             {{{{{1, 2}}, {{3, 4}}}, 1}, {{{{5, 6}}, {{7, 8}}}, 4}};
//         auto ndim = itertools::detail::get_ndim(iter);
//         std::string s = itertools::to_string(iter);
//         if (print) {
//             std::cout << s << std::endl;
//         }
//     }
//     {
//         std::vector<std::list<std::vector<std::list<std::vector<std::deque<int>>>>>>
//             iter =
//                 {{{{{{0, 1}, {2, 3}},

//                     {{4, 5}, {6, 7}}},

//                    {{{8, 9}, {10, 11}},

//                     {{12, 13}, {14, 15}}}},

//                   {{{{16, 17}, {18, 19}},

//                     {{20, 21}, {22, 23}}},

//                    {{{24, 25}, {26, 27}},

//                     {{28, 29}, {30, 31}}}}},

//                  {{{{{32, 33}, {34, 35}},

//                     {{36, 37}, {38, 39}}},

//                    {{{40, 41}, {42, 43}},

//                     {{44, 45}, {46, 47}}}},

//                   {{{{48, 49}, {50, 51}},

//                     {{52, 53}, {54, 55}}},

//                    {{{56, 57}, {58, 59}},

//                     {{60, 61}, {62, 63}}}}}};
//         auto ndim = itertools::detail::get_ndim(iter);
//         std::string s = itertools::to_string(iter);
//         if (print) {
//             std::cout << s << std::endl;
//         }
//     }
//     {
//         std::vector<std::tuple<
//             std::list<std::vector<std::vector<int>>>,
//             int,
//             std::map<int, std::tuple<int, int, int>>>>
//             iter =
//                 {{{{{1, 2}}, {{3, 4}}},
//                   1,
//                   {{1, {0, 1, 2}}, {2, {1, 2, 3}}, {3, {2, 3, 4}}, {4, {3, 4, 5}}}},
//                  {{{{5, 6}}, {{7, 8}}},
//                   4,
//                   {{1, {0, 1, 2}}, {2, {1, 2, 3}}, {3, {2, 3, 4}}, {4, {3, 4, 5}}}}};
//         auto ndim = itertools::detail::get_ndim(iter);
//         std::string s = itertools::to_string(iter);
//         if (print) {
//             std::cout << s << std::endl;
//         }
//     }
//     {
//         std::vector<std::vector<int>> iter =
//             {{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
//               17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
//               34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
//               51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
//               68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
//               85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99},
//              {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16,
//               17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
//               34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
//               51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67,
//               68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
//               85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99}};
//         auto ndim = itertools::detail::get_ndim(iter);
//         std::string s = itertools::to_string(iter);
//         if (print) {
//             std::cout << s << std::endl;
//         }
//     }
// }

void
range_container_tests()
{
    using namespace itertools::views;
    using namespace itertools;

    {
        std::vector<int> v1 = { 1, 2, 3, 4, 5 };
        std::vector<int> v2 = { 10, 9, 8, 7, 6 };
        std::vector<int> v3 = { 100, 200, 300, 400, 500 };

        auto l1 = std::list<float>{ 1.2, 1.2, 1.2 };
        auto l2 = std::list<float>{ 9.9, 8.8, 7.7 };

        auto v1_enum = views::enumerate(v1);
        auto v2_enum = views::enumerate(v2);

        auto f = [](auto v) { return v; };

        auto zipped = zip(v1, v1_enum, zip(zip(zip(zip(l1), v2))));

        for (auto&& t : zipped) {
            auto& [i, tup, tt] = t;
            auto& [n, j] = tup;
            std::cout << fmt::format("{}, {}", n, j) << std::endl;
        }
    }
}

void
test_reverse()
{
    using namespace itertools;
    using namespace std::string_literals;

    std::vector<int> v;
    for (auto i : views::iota(13)) {
        v.push_back(i);
    }

    std::vector<int> expected = { 0, 2, 4, 6, 8, 10, 12 };
    std::vector<int> r_expected = { 12, 10, 8, 6, 4, 2, 0 };

    auto p = [pos = 0](auto&& x) { return x % 2 == 0; };

    auto rng = v | views::filter(p) | views::reverse() | views::filter(p) |
               views::reverse() | views::filter(p) | views::reverse() |
               views::filter(p) | views::reverse() | views::filter(p) |
               views::reverse() | views::filter(p) | views::reverse();

    assert(equal(rng, expected));
    assert(equal(rng | views::reverse(), r_expected));

    auto s = "Amami, Alfredo, quant'io t'amo."s;
    auto r_s = s | views::reverse() | itertools::to<std::string>();

    assert(equal(r_s, ".oma't oi'tnauq ,oderflA ,imamA"s));
}

void
test_filter()
{
    using namespace itertools;

    std::vector<int> v = { 2, 2, 2, 2, 3, 4, 7, 8, 9, 2, 2, 2, 2, 2 };
    std::vector<int> expected = { 3, 4, 7, 8, 9 };

    auto p = [](auto&& x) { return x == 2; };

    auto rng = v | views::drop_while(p) | views::reverse() | views::drop_while(p) |
               views::reverse();

    for (auto&& i : rng) {
        std::cout << i << std::endl;
    }

    assert(equal(rng, expected));
}

void
test_zip()
{
    using namespace itertools;

    std::vector<int> v1 = { 9, 8, 7, 6, 5 };
    std::vector<int> v2 = { 2, 2, 3, 4, 7, 8, 9, 2, 2, 2 };
    std::vector<std::unique_ptr<int>> v3;

    std::vector<std::tuple<int, int, int>> expected = { { 9, 2, 1 },
                                                        { 8, 2, 2 },
                                                        { 7, 3, 3 } };
    int a = 1;
    int b = 2;
    int c = 3;
    v3.push_back(std::make_unique<int>(a));
    v3.emplace_back(std::make_unique<int>(b));
    v3.emplace_back(std::make_unique<int>(c));

    auto rng = views::zip(v1, v2, v3);

    for (auto&& [x, y, z] : rng) {
    }
    for (auto&& [x, y, z] : rng | views::reverse()) {
    }

    auto v4 = v3 | views::transform([](auto&& x) { return *x.get(); });
    auto rng2 = views::zip(v1, v2, v4);

    assert(equal(rng2, expected, false));
}

void
test_concat()
{
    std::vector<int> expected = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    auto r_expected = expected | views::reverse() | itertools::to<std::vector>();

    std::vector<int> v1 = { 1, 2, 3, 4, 5 };
    std::vector<int> v2 = { 6, 7, 8, 9, 10 };
    std::vector<int> v3 = { 11, 12, 13, 14, 15 };

    auto rng = views::concat(v1, v2, v3);

    assert(equal(rng, expected));
    assert(equal(rng | views::reverse(), r_expected));

    auto rng2 = rng | views::reverse() | views::reverse() | views::reverse() |
                views::reverse() | views::reverse() | views::reverse() |
                views::reverse() | views::reverse();

    assert(equal(rng2, expected));
    assert(equal(rng2 | views::reverse(), r_expected));
}

auto trim_front = views::drop_while([](char c) { return std::isspace(c); });

auto trim_back = views::reverse() | trim_front | views::reverse();

constexpr auto trim = [](std::string s) {
    return s | trim_front | trim_back | to<std::string>();
};

int
main()
{
    using namespace std::string_literals;
    using namespace itertools;

    zip_tests();
    any_tests();
    enumerate_tests();
    // range_tests();
    // itertools_tests();
    tupletools_tests();
    // to_string_tests();

    range_container_tests();

    test_reverse();
    test_filter();
    test_zip();
    test_concat();

    auto s = "    for the love of      "s | trim |
             views::transform([](char x) { return std::toupper(x); }) |
             to<std::string>();

    std::cout << s << std::endl;

    std::vector<int> v1 = { 1, 2, 3, 4, 5 };
    std::vector<int> v2 = { 10, 9, 8, 7, 6 };
    std::vector<int> v3 = { 100, 200, 300, 400, 500 };

    auto rng2 = v1 | views::block(2);

    for (auto i : rng2) {
        std::cout << i.size() << std::endl;
    }

    std::vector<std::tuple<std::list<std::vector<std::vector<int>>>,
                           int,
                           std::map<int, std::tuple<int, int, int>>>>
      iter = { { { { { 1, 2 } }, { { 3, 4 } } },
                 1,
                 { { 1, { 0, 1, 2 } },
                   { 2, { 1, 2, 3 } },
                   { 3, { 2, 3, 4 } },
                   { 4, { 3, 4, 5 } } } },
               { { { { 5, 6 } }, { { 7, 8 } } },
                 4,
                 { { 1, { 0, 1, 2 } },
                   { 2, { 1, 2, 3 } },
                   { 3, { 2, 3, 4 } },
                   { 4, { 3, 4, 5 } } } } };

    // std::vector<std::vector<std::vector<int>>> iter =
    //     {{{1, 2, 3}, {4, 5, 6}},
    //      {{7, 8, 9}, {10, 11, 12}},
    //      {{13, 14, 15}, {16, 17, 18}},
    //      {{19, 20, 21}, {22, 23, 24}}};

    // std::vector<std::tuple<std::vector<int>, int>> iter = {{{1, 2}, 1}, {{2, 3}, 2}};

    // auto ndim = itertools::detail::get_ndim(iter);
    // std::string s = itertools::to_string(iter);

    // std::cout << s << std::endl;

    fmt::print("tests complete\n");
    return 0;
}