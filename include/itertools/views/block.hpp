#include "iota.hpp"
#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"

#pragma once

namespace itertools {
namespace views {

namespace detail {

template<class Range>
class block_container : public cached_container<Range>
{
  public:
    template<class Iter>
    class iterator : public range_iterator<Iter>
    {
        block_container* base;
        std::vector<std::remove_cvref_t<iter_value_t<Iter>>> ret;

      public:
        iterator(block_container* base, Iter&& it) noexcept
          : range_iterator<Iter>(std::forward<Iter>(it))
          , base(base)
        {
            ret.reserve(base->block_size);
        }

        bool is_complete() { return *base->begin_ == *base->end_; }

        auto operator++()
        {
            for (auto i : views::iota(ret.size())) {
                ret.pop_back();
            }
            return *this;
        }

        auto operator*() noexcept
        {
            for (auto i : views::iota(base->block_size)) {
                if (is_complete()) {
                    break;
                } else {
                    ret.push_back(*this->it);
                    ++this->it;
                }
            }
            return ret;
        }
    };

    template<class Iter>
    iterator(block_container*, Iter&&) -> iterator<Iter>;

    size_t block_size;

    block_container(Range&& range, size_t block_size)
      : cached_container<Range>{ std::forward<Range>(range) }
      , block_size(block_size)
    {}

    auto begin() { return iterator(this, this->cache_begin()); }

    auto end() { return iterator(this, this->cache_end()); }
};

template<class Range>
block_container(Range&&) -> block_container<Range>;

template<class Range>
constexpr block_container<Range>
block(Range&& range, size_t block_size)
{
    return block_container<Range>(std::forward<Range>(range), block_size);
};
}

constexpr auto
block(size_t block_size)
{
    return [=]<class Range>(Range&& range) {
        return detail::block(std::forward<Range>(range), block_size);
    };
}

}
}