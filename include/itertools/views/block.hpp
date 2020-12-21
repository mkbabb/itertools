#include <itertools/range_iterator.hpp>

namespace itertools { namespace views {

#pragma once

template<class Range>
class block_iterator : public range_iterator<Range>
{
public:
    using Value = tupletools::range_value_t<Range>;
    std::vector<Value> ret;

    block_iterator(Range&& range, size_t block_size) noexcept
      : range_iterator<Range>(std::forward<Range>(range))
    {
        this->block_size = block_size;
        ret.reserve(block_size);
    }

    auto operator++()
    {
        auto i = 0;
        while (!this->is_complete() && i++ < this->block_size) {
            ret.pop_back();
        }
        return *this;
    }

    auto operator*() noexcept
    {
        auto i = 0;

        while (!this->is_complete() && i++ < this->block_size) {
            ret.push_back(*(this->begin_it));
            ++(this->begin_it);
        }

        return ret;
    }

    size_t block_size;
};

// template<class Func, class Range>
// constexpr auto
// block(Range&& range, size_t block_size)
// {
//     auto it = block_iterator(std::forward<Range>(range), block_size);
//     using Iterator = decltype(it);

//     return range_container<
//         Range,
//         Iterator>(std::forward<Range>(range), std::forward<Iterator>(it));
// };

}}