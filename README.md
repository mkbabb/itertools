# itertools

A simple iterator library for C++, inspired by the great
[`range/v3`](https://github.com/ericniebler/range-v3) and Python.

Though this technically _is_ a C++20 library, it only supports the current feature set
offered by clang 11.0. Of note, there is no usage of `std::ranges`, or `concepts`.

- [itertools](#itertools)
  - [Quick Example](#quick-example)
  - [Why?](#why)
  - [How; Functionality](#how-functionality)
  - [Iterator containers and views](#iterator-containers-and-views)
    - [The base iterator container](#the-base-iterator-container)
    - [The iterators](#the-iterators)
    - [The views](#the-views)
  - [The rest](#the-rest)
      - [Infinite iterators](#infinite-iterators)
      - [Terminating iterators](#terminating-iterators)
      - [Combinatorics](#combinatorics)
  - [Tupletools](#tupletools)
  - [Use these instead](#use-these-instead)

## Quick Example

Create a calendar for all months in the ... _whoops_ wrong library.

```c++
...
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
...
```

See [example.cpp](tests/example.cpp) for the full example.

## Why?

Primarily as an academic endeavour into the, ahem, _interesting_ side of cpp's
iterators, copy, value and move operations, and all things perfect forwarding related.
It also serves as sort of a template for using the [`conan`](https://conan.io) package
manager.

## How; Functionality

Common in Python is the generous usage of iterators and generators (the latter of which
is out of scope for this library; see
[`cppcoro`](https://github.com/lewissbaker/cppcoro) for more). Things like `zip`,
`range`, `enumerate` pop up everywhere in (my) Python code: how difficult would it be to
bring that functionality to C++? The answer is: not that difficult at all! With a
heavy-handed usage of templates, SFINAE (clang plz finish concepts), and lots of
`std::tuple`s, we can achieve just that.

## Iterator containers and views

### The base iterator container

We encapsulate a `range`-like thing into a `range_container`: an object with a container
of some sort (it can be another `range_container`, too), and an iterator that's used to
traverse said container. It also supports the prototypical `operator|` for simple
chaining of range functions.

### The iterators

The iterator used inside of a `range_container` is almost a standard C++ iterator, but
with a few differences:

-   Abstracts away the `operator==` logic to use a sentinel value;
    `range_container_terminus`.
-   Holds the original range + its begin and end iterators (defaulted to a `std::begin`
    and `std::end` of the input range).
-   provides an extra stop signaling method, `is_complete`, intended to be (potentially)
    overridden by subclassed iterator.

### The views

The 'views' are really just the special iterators used for traversing a range. Most are
rather uninteresting and do exactly what you'd think: `filter` filters by a predicate,
`transform` transforms by a function, etc.

`zip`, which behaves almost identically to Python's `zip`, _is_ interesting, however:
zips together an arbitrary number of ranges (yes, you can zip a zip et infinitum) and
lazily returns a tuple of each argument. If the input ranges are not of homogenous size,
it'll clamp iteration to the smallest input range (a la Python). Important to note that
`zip` returns a tuple of **references** to the input range values. To my knowledge,
there is no way to impose both a reference and copy operation onto a tuple using
assignment (meaning `auto x = std::make_tuple(...)` _always_ copies, and
`auto x = std::forward_as_tuple(...)` _always_ references); it's a binary thing, so
returning a reference was chosen.

Which will lead us to `tupletools` in just a moment.

## The rest

A few other Python itertools niceties were implemented/are being implemented.

Here's a list of what's currently available:

#### Infinite iterators

-   [ ] `count`
-   [ ] `cycle`
-   [ ] `repeat`

#### Terminating iterators

-   [ ] `accumulate`
-   [x] `chain`
-   [ ] `chain.from_iterable`
-   [ ] `compress`
-   [x] `dropwhile; filterfalse` (using `views::filter`)
-   [ ] `groupby`
-   [x] `slice` (using `views::slice` and `views::stride`)
-   [ ] `takewhile`
-   [ ] `tee`
-   [ ] `zip_longest`

#### Combinatorics

-   [ ] `product`
-   [ ] `permutations`
-   [ ] `combinations`
-   [ ] `combinations_with_replacement`

## Tupletools

This header provides a series of abstractions to make tuple manipulation easier. The
driver behind nearly everything is a tuple version of `for_each`, which takes in a
function and applies it to every argument of the tuple. Similarly to `JavaScript`
(yeah), the callback function takes both the current index and the current value. Saving
the index can allow for some pretty interesting functionality, like `tupletools::roll`,
which rolls a tuple either left or right.

## Use these instead

Several libraries accomplish this library's purpose and **more**. This really isn't
meant for production. Here are a few I've enjoyed:

-   [`range/v3`](https://github.com/ericniebler/range-v3)
-   [`NanoRange`](https://github.com/tcbrindle/NanoRange)
