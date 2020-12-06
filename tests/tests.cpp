#define FMT_HEADER_ONLY

#include "../src/itertools.hpp"
#include "../src/math.hpp"
#include "fmt/format.h"
#include "random_v/random_v.hpp"

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

void
zip_tests()
{
    {
        std::vector<int> iv1 = {101, 102, 103, 104};
        std::vector<int> iv2 = {9, 8, 7, 6, 5, 4, 3, 2};
        std::vector<int> iv3 = {123, 1234, 12345};

        for (auto [i, j, k] : itertools::zip(iv1, iv2, iv3)) {
        };
    }
    {
        std::vector<std::string> sv1 =
            {"\nThis", "but", "different", "containers,"};
        std::vector<std::string> sv2 = {"is", "can", "types", "too?"};
        std::vector<std::string> sv3 =
            {"cool,", "we iterate through", "of", "...\n"};

        for (auto [i, j, k] : itertools::zip(sv1, sv2, sv3)) {
        };
    }
    {

        std::list<std::string> sl1 =
            {"Yes, we can!", "Some more numbers:", "More numbers!"};
        std::vector<std::string> sv1 =
            {"Different types, too!", "Ints and doubles.", ""};
        std::list<int> iv1 = {1, 2, 3};
        std::vector<double> dv1{3.141592653589793238, 1.6181, 2.71828};

        for (auto [i, j, k, l] : itertools::zip(sl1, sv1, iv1, dv1)) {
        }
    }
    {
        std::map<int, int> id1 =
            {{0, 10}, {1, 11}, {2, 12}, {3, 13}, {4, 14}, {5, 15}};
        std::list<std::string> sv1 =
            {"1", "mijn", "worten", "2", "helm", "dearth"};
        std::vector<double> dv1 = {1.2, 3.4, 5.6, 6.7, 7.8, 8.9, 9.0};

        for (auto [i, j, k, l] :
             itertools::zip(id1, sv1, dv1, itertools::range(7))) {
            auto [key, value] = i;
            fmt::print("{}: {}, {}, {}, {}\n", key, value, j, k, l);
        }
    }
}

void
tupletools_tests()
{
    {
        auto tup1 = std::make_tuple(1, 2, 3, 4);
        assert(tupletools::to_string(tup1) == "(1, 2, 3, 4)");
    }
    {
        auto tup1 = tupletools::make_tuple_of<20>(true);
        assert(tupletools::tuple_size<decltype(tup1)>::value == 20);
    }
    {
        std::vector<std::tuple<int, int>> tv1 =
            {{1, 2}, {3, 4}, {5, 6}, {7, 8}};
        for (auto v : itertools::enumerate(tv1)) {
            auto [n, i, j] = tupletools::flatten(v);
            fmt::print("{} {} {}\n", n, i, j);
        }
    }
    {
        auto tup1 = std::make_tuple('a', 'b', 'c', 'd');
        tup1 = tupletools::roll(tup1);
        assert(tup1 == std::make_tuple('d', 'a', 'b', 'c'));
    }
}

void
itertools_tests()
{
    {
        std::vector<std::string> sv1 = {"h", "e", "l", "l", "o"};
        std::vector<int> iv1 = {1, 2, 3, 4, 5, 6, 7, 8};

        assert(itertools::join(sv1, "") == "hello");
        assert(itertools::join(sv1, ",") == "h,e,l,l,o");
        assert(itertools::join(iv1, ", ") == "1, 2, 3, 4, 5, 6, 7, 8");
    }
    {
        std::vector<int> iv1 = {1, 2, 3, 4, 5, 6, 7, 8};
        itertools::roll(itertools::roll(iv1));
        assert(iv1 == (std::vector<int>{7, 8, 1, 2, 3, 4, 5, 6}));
    }
}

void
any_tests()
{
    {
        // Any tests with initializer list
        auto tup1 = std::make_tuple(1, 2, 3, 4, 5, 6);
        auto tup2 = std::make_tuple(33, 44, 77, 4, 5, 99);

        auto tup_bool = tupletools::
            where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);
        bool bool1 = tupletools::any_of(tup_bool);
        bool bool2 = tupletools::
            any_where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);

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

        auto tup_bool = tupletools::
            where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);
        bool bool1 = tupletools::all_of(tup_bool);
        bool bool2 = tupletools::
            all_where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);

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

        auto ilist = tupletools::
            where([](auto&& x, auto&& y) { return x == y; }, tup1, tup2);

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
    auto tup = std::make_tuple(1, 2);
    {
        std::vector<int> v1(100000, 0);
        int j = 0;
        int k = 0;
        for (auto [n, i] : itertools::enumerate(v1)) {
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

        for (auto [n, i] : itertools::enumerate(v1)) {
            j++;
            k = n;
            std::get<0>(tup);
        }
        assert((j - 1) == k);
    }
}

void
range_tests()
{
    {
        int stop = -999'999;
        int j = stop;
        auto _range = itertools::range(stop);
        for (auto i : _range) {
            assert(i == j);
            j++;
        }
        assert(j == 0);
    }
    {
        int stop = 1'999'999;
        int j = 0;
        auto _range = itertools::range(stop);
        for (auto i : _range) {
            assert(i == j);
            j++;
        }
        assert(j == stop);
    }

    {
        int stop = -999'999;
        int j = stop;
        auto _range = itertools::range(stop, 0);
        for (auto i : _range) {
            assert(i == j);
            j++;
        }
        assert(j == 0);
    }
}

void
reduction_tests()
{
    {
        std::vector<std::tuple<int, int>> iter =
            {{0, 1}, {1, 2}, {3, 4}, {5, 6}, {7, 8}};

        int sm = itertools::reduce<int>(iter, 0, [](auto n, auto v, auto i) {
            return std::get<0>(v) + i;
        });

        assert(sm == 16);

        int ml = itertools::reduce<int>(iter, 1, [](auto n, auto v, auto i) {
            return std::get<1>(v) * i;
        });

        assert(ml == 384);
    }
    {
        std::vector<int> iter = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                                 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
                                 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                                 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
                                 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83,
                                 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
                                 96, 97, 98, 99};

        int sm = itertools::sum<int>(iter);

        assert(sm == 4950);
    }
}

void
time_multiple_tests()
{
    size_t N = 1'000;
    {
        size_t M = 10'000;

        auto func1 = [&]() {
            size_t t = 0;
            std::string h = "";
            for (auto i : itertools::range(M)) {
                h = std::to_string(i);
            };
            return t;
        };

        auto func2 = [&]() {
            size_t t = 0;
            std::string h = "";
            for (size_t i = 0; i < M; i++) {
                h = std::to_string(i);
            };
            return t;
        };

        auto [times, extremal_times] =
            itertools::time_multiple(N, func1, func2);
        for (auto [key, value] : times) {
            fmt::print("function {}:\n", key);
            for (auto i : extremal_times[key]) {
                fmt::print("\t{}\n", i);
            }
        }
    }
    {
        size_t M = 10'000;

        auto func1 = [&]() {
            std::vector<int> iv1(M, 1);
            std::string h = "";
            for (auto [n, i] : itertools::enumerate(iv1)) {
                h = std::to_string(n);
                h = std::to_string(i);
            };
        };

        auto func2 = [&]() {
            std::vector<int> iv1(M, 1);
            std::string h = "";
            for (auto [n, i] :
                 itertools::zip(itertools::range(iv1.size()), iv1)) {
                h = std::to_string(n);
                h = std::to_string(i);
            };
        };

        auto func3 = [&]() {
            std::vector<int> iv1(M, 1);
            std::string h = "";
            size_t n = 0;
            for (auto i : iv1) {
                h = std::to_string(n);
                h = std::to_string(i);
                n++;
            };
        };

        auto [times, extremal_times] =
            itertools::time_multiple(N, func1, func2, func3);
        for (auto [key, value] : times) {
            fmt::print("function {}:\n", key);
            for (auto i : extremal_times[key]) {
                fmt::print("\t{}\n", i);
            }
        }
    }
}

void
to_string_tests(bool print = false)
{
    {
        auto iter = std::make_tuple(1, 2, 3, 4, 5, 6);
        auto ndim = itertools::get_ndim(iter);
        std::string s = itertools::to_string(iter);
        if (print) {
            std::cout << s << std::endl;
        }
    }
    {
        auto iter = std::make_tuple(
            std::make_tuple(1, 2, 3, 4, 5, 6),
            std::make_tuple(1, 2, 3, 4, 5, 6),
            std::make_tuple(1, 2, 3, 4, 5, 6));
        auto ndim = itertools::get_ndim(iter);
        std::string s =
            itertools::to_string_f(iter, [](auto&& v) -> std::string {
                return " " + std::to_string(v) + " ";
            });
        if (print) {
            std::cout << s << std::endl;
        }
    }
    {
        std::vector<std::map<int, int>> iter =
            {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}};
        auto ndim = itertools::get_ndim(iter);
        std::string s = itertools::to_string(iter);
        if (print) {
            std::cout << s << std::endl;
        }
    }
    {
        std::vector<std::tuple<std::vector<std::vector<std::vector<int>>>, int>>
            iter = {{{{{1, 2}}, {{3, 4}}}, 1}, {{{{5, 6}}, {{7, 8}}}, 4}};
        auto ndim = itertools::get_ndim(iter);
        std::string s = itertools::to_string(iter);
        if (print) {
            std::cout << s << std::endl;
        }
    }
    {
        std::vector<
            std::list<std::vector<std::list<std::vector<std::deque<int>>>>>>
            iter =
                {{{{{{0, 1}, {2, 3}},

                    {{4, 5}, {6, 7}}},

                   {{{8, 9}, {10, 11}},

                    {{12, 13}, {14, 15}}}},

                  {{{{16, 17}, {18, 19}},

                    {{20, 21}, {22, 23}}},

                   {{{24, 25}, {26, 27}},

                    {{28, 29}, {30, 31}}}}},

                 {{{{{32, 33}, {34, 35}},

                    {{36, 37}, {38, 39}}},

                   {{{40, 41}, {42, 43}},

                    {{44, 45}, {46, 47}}}},

                  {{{{48, 49}, {50, 51}},

                    {{52, 53}, {54, 55}}},

                   {{{56, 57}, {58, 59}},

                    {{60, 61}, {62, 63}}}}}};
        auto ndim = itertools::get_ndim(iter);
        std::string s = itertools::to_string(iter);
        if (print) {
            std::cout << s << std::endl;
        }
    }
    {
        std::vector<std::tuple<
            std::list<std::vector<std::vector<int>>>,
            int,
            std::map<int, std::tuple<int, int, int>>>>
            iter =
                {{{{{1, 2}}, {{3, 4}}},
                  1,
                  {{1, {0, 1, 2}},
                   {2, {1, 2, 3}},
                   {3, {2, 3, 4}},
                   {4, {3, 4, 5}}}},
                 {{{{5, 6}}, {{7, 8}}},
                  4,
                  {{1, {0, 1, 2}},
                   {2, {1, 2, 3}},
                   {3, {2, 3, 4}},
                   {4, {3, 4, 5}}}}};
        auto ndim = itertools::get_ndim(iter);
        std::string s = itertools::to_string(iter);
        if (print) {
            std::cout << s << std::endl;
        }
    }
    {
        std::vector<std::vector<int>> iter =
            {{0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
              15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
              30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
              45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
              60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
              75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
              90, 91, 92, 93, 94, 95, 96, 97, 98, 99},
             {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
              15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
              30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
              45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
              60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
              75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
              90, 91, 92, 93, 94, 95, 96, 97, 98, 99}};
        auto ndim = itertools::get_ndim(iter);
        std::string s = itertools::to_string(iter);
        if (print) {
            std::cout << s << std::endl;
        }
    }
}

template<typename T>
constexpr bool
assert_almost_equal(T v1, T v2, T epsilon = 1e-12)
{
    auto delta = abs(v1 - v2);
    if (delta < epsilon) {
        return true;
    } else {
        return false;
    }
}

void
exp_tests()
{
    random_v::Random rng(0ULL, random_v::lcg_xor_rot);
    int a = -100;
    int b = 100;

    for (double i : itertools::range(100'000)) {
        double d = static_cast<double>(rng.randrange(a, b)) / pi;

        auto v1 = expc(d);
        auto v2 = exp(d);

        bool b = assert_almost_equal(v1, v2, 1e-4);

        if (!b) {
            throw 2;
        }
    }
}

void
pow_tests()
{
    random_v::Random rng(0ULL, random_v::lcg_xor_rot);
    int a1 = 1;
    int b1 = 100;

    int a2 = 1;
    int b2 = 10;

    for (double i : itertools::range(100'000)) {
        double x = static_cast<double>(rng.randrange(a1, b1)) / pi;
        double a = static_cast<double>(rng.randrange(a2, b2)) / pi;

        int p = 0;

        auto v = frexp(x, &p);

        fmt::print("{}, {}, {}", x, p, v);

        auto v1 = powc(x, a);
        auto v2 = pow(x, a);
        fmt::print("{:.52f}\n{:.52f}\n", v1, v2);
    }
}

union double_bits
{
    double value;
    uint64_t ivalue;

    constexpr double_bits(double d)
      : value(d){};
};

std::tuple<uint, int, uint64_t>
frexp0(double d)
{
    uint sign = 0;
    int exponent;

    double dmantissa = frexp(d, &exponent);

    if (dmantissa < 0) {
        dmantissa *= -1;
        sign = 1;
    }

    uint64_t mantissa = dmantissa * two53;

    return {sign, exponent, mantissa};
}

constexpr std::tuple<uint, int, uint64_t>
frexp1(double d)
{
    uint64_t ivalue = 0;

    double_bits db(d);

    uint sign = db.ivalue >> 63;
    int exponent = (db.ivalue << 1) >> 53;
    uint64_t mantissa = (db.ivalue << 12) >> 12;

    if (exponent == 0) {
        exponent++;
    } else {
        mantissa |= (1L << 52);
    }
    exponent -= 1022;

    return {sign, exponent, mantissa};
}

std::tuple<uint, int, uint64_t>
frexp3(double d)
{
    uint64_t ivalue = *((uint64_t*) (&d));

    double mantissa = 1.0;

    uint sign = ivalue >> 63;
    int exponent = (int) ((ivalue >> 52) & 0x7ffL);
    uint64_t imantissa = ivalue & 0xfffffffffffffL;

    if (exponent == 0) {
        exponent++;
    } else {
        imantissa |= (1L << 52);
    }

    // bias the exponent - actually biased by 1023.
    // we are treating the mantissa as m.0 instead of 0.m
    //  so subtract another 52.
    exponent -= 1075;
    mantissa = imantissa;
    uint64_t mant = imantissa;

    // normalize
    while (mantissa >= 1.0) {
        imantissa >>= 1;
        mantissa /= 2.;
        exponent++;
    }

    if (sign) {
        mantissa *= -1;
    }

    return {sign, exponent, mant};
}

void
frexp_tests()
{
    random_v::Random rng(0ULL, random_v::lcg_xor_rot);
    int a = -100'000;
    int b = 100'000;

    for (double i : itertools::range(100'000)) {
        double d = static_cast<double>(rng.randrange(a, b)) / 10000.0;

        auto tup0 = cfrexp(d);
        auto tup1 = frexp0(d);
        auto tup2 = frexp1(d);
        auto tup3 = frexp3(d);

        assert(tup0 == tup1);
        assert(tup0 == tup2);
        assert(tup0 == tup3);
    }
}

void
range_container_tests()
{
    using namespace itertools;
    {
        std::vector<int> v1 = {1, 2, 3, 4, 5};
        std::vector<int> v2 = {10, 9, 8, 7, 6};
        std::vector<int> v3 = {100, 200, 300, 400, 500};

        auto l1 = std::list<float>{1.2, 1.2, 1.2};
        auto l2 = std::list<float>{9.9, 8.8, 7.7};

        auto v1_enum = itertools::enumerate(v1);
        auto v2_enum = itertools::enumerate(v2);

        auto f = [](auto v) { return v; };

        auto zipped = itertools::zip_ref(
            v1,
            v1_enum,
            itertools::zip(
                itertools::zip(itertools::zip(itertools::enumerate(l1), v2))));

        for (auto t : zipped | itertools::piper(f, itertools::transmog)) {
            auto& [i, tup, tt] = t;
            auto& [n, j] = tup;
            std::cout << fmt::format("{}, {}", n, j) << std::endl;
        }

        auto v4 = itertools::map(std::vector<int>(10), [](auto n, auto v) {
            return 999;
        });

        auto pred = itertools::
            piper([](auto v) { return std::get<0>(v) > 3; }, itertools::filter);

        auto zp = itertools::zip(v1, v2) | pred;

        std::cout << itertools::to_string(zp) << std::endl;
    }
    {
        std::vector<int> v1 = {1, 2, 3, 4, 5};
        auto v2 = itertools::
            map(std::vector<int>(10), itertools::spawn(itertools::range(3)));

        std::cout << itertools::to_string(v2) << std::endl;
    }
}

int
main()
{

    // zip_tests();
    // any_tests();
    // enumerate_tests();
    // range_tests();
    // itertools_tests();
    // tupletools_tests();
    // reduction_tests();
    // generator_tests();
    // time_multiple_tests();
    // to_string_tests();
    // frexp_tests();

    // range_container_tests();

    std::vector<int> v1 = {1, 5, 9, 2, 8, 77};

    auto r1 = itertools::zip(v1, v1);

    // auto piper = itertools::piper(
    //     [](auto v) { return itertools::to_string(v); }, itertools::transmog);

    // auto range = itertools::to_range(v1) | [](auto v) {
    //     auto vec = itertools::to_vector(v);
    //     std::sort(begin(vec), end(vec));
    //     return itertools::to_range(vec);
    // };

    // auto range =
    //     itertools::to_range(v1) | itertools::piper(
    //                                            [](auto&& v) {
    //                                                std::sort(begin(v),
    //                                                end(v)); return v;
    //                                            },
    //                                            itertools::transmog_hasty);

    // for (auto i : range) {
    //     std::cout << i << std::endl;
    // }

    fmt::print("tests complete\n");
    return 0;
}