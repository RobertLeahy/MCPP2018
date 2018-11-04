#include <mcpp/prefix_buffer_sequence.hpp>

#include <algorithm>
#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

template<typename T,
         bool Const>
constexpr bool test = std::is_same_v<typename std::iterator_traits<decltype(boost::asio::buffer_sequence_begin(std::declval<const prefix_buffer_sequence<T>&>()))>::value_type,
                                     std::conditional_t<Const,
                                                        boost::asio::const_buffer,
                                                        boost::asio::mutable_buffer>>;

static_assert(test<boost::asio::const_buffer,
                   true>);
static_assert(test<boost::asio::mutable_buffer,
                   false>);
static_assert(boost::asio::is_const_buffer_sequence<prefix_buffer_sequence<boost::asio::const_buffer>>::value);
static_assert(!boost::asio::is_mutable_buffer_sequence<prefix_buffer_sequence<boost::asio::const_buffer>>::value);
static_assert(boost::asio::is_const_buffer_sequence<prefix_buffer_sequence<boost::asio::mutable_buffer>>::value);
static_assert(boost::asio::is_mutable_buffer_sequence<prefix_buffer_sequence<boost::asio::mutable_buffer>>::value);

TEST_CASE("prefix_buffer_sequence w/ConstBufferSequence",
          "[mcpp][prefix_buffer_sequence]")
{
  const char buffer[] = {1, 2, 3, 4, 5};
  boost::asio::const_buffer cb(buffer,
                               sizeof(buffer));
  SECTION("All") {
    prefix_buffer_sequence pbs(cb,
                               sizeof(buffer));
    auto iter = boost::asio::buffer_sequence_begin(pbs);
    REQUIRE(iter != boost::asio::buffer_sequence_end(pbs));
    std::optional<boost::asio::const_buffer> cb(*iter);
    CHECK(cb->size() == sizeof(buffer));
    CHECK(cb->data() == buffer);
    ++iter;
    CHECK(iter == boost::asio::buffer_sequence_end(pbs));
    --iter;
    REQUIRE(iter != boost::asio::buffer_sequence_end(pbs));
    cb.emplace(*iter);
    CHECK(cb->size() == sizeof(buffer));
    CHECK(cb->data() == buffer);
    CHECK(boost::asio::buffer_size(pbs) == sizeof(buffer));
    const char* begin = buffer;
    const char* end = begin + sizeof(buffer);
    CHECK(std::equal(begin,
                     end,
                     boost::asio::buffers_begin(pbs),
                     boost::asio::buffers_end(pbs)));
    CHECK(std::equal(std::reverse_iterator(end),
                     std::reverse_iterator(begin),
                     std::reverse_iterator(boost::asio::buffers_end(pbs)),
                     std::reverse_iterator(boost::asio::buffers_begin(pbs))));
  }
  SECTION("Non-empty prefix") {
    prefix_buffer_sequence pbs(cb,
                               sizeof(buffer) - 1);
    auto iter = boost::asio::buffer_sequence_begin(pbs);
    REQUIRE(iter != boost::asio::buffer_sequence_end(pbs));
    std::optional<boost::asio::const_buffer> cb(*iter);
    CHECK(cb->size() == (sizeof(buffer) - 1));
    CHECK(cb->data() == buffer);
    ++iter;
    CHECK(iter == boost::asio::buffer_sequence_end(pbs));
    --iter;
    REQUIRE(iter != boost::asio::buffer_sequence_end(pbs));
    cb.emplace(*iter);
    CHECK(cb->size() == (sizeof(buffer) - 1));
    CHECK(cb->data() == buffer);
    CHECK(boost::asio::buffer_size(pbs) == (sizeof(buffer) - 1));
    const char* begin = buffer;
    const char* end = begin + sizeof(buffer) - 1;
    CHECK(std::equal(begin,
                     end,
                     boost::asio::buffers_begin(pbs),
                     boost::asio::buffers_end(pbs)));
    CHECK(std::equal(std::reverse_iterator(end),
                     std::reverse_iterator(begin),
                     std::reverse_iterator(boost::asio::buffers_end(pbs)),
                     std::reverse_iterator(boost::asio::buffers_begin(pbs))));
  }
  SECTION("Empty prefix") {
    prefix_buffer_sequence pbs(cb,
                               0);
    auto iter = boost::asio::buffer_sequence_begin(pbs);
    CHECK(iter == boost::asio::buffer_sequence_end(pbs));
    CHECK(boost::asio::buffer_size(pbs) == 0);
    const char* begin = buffer;
    const char* end = begin;
    CHECK(std::equal(begin,
                     end,
                     boost::asio::buffers_begin(pbs),
                     boost::asio::buffers_end(pbs)));
    CHECK(std::equal(std::reverse_iterator(end),
                     std::reverse_iterator(begin),
                     std::reverse_iterator(boost::asio::buffers_end(pbs)),
                     std::reverse_iterator(boost::asio::buffers_begin(pbs))));
  }
}

TEST_CASE("prefix_buffer_sequence w/MutableBufferSequence",
          "[mcpp][prefix_buffer_sequence]")
{
  char buffer[] = {1, 2, 3, 4, 5};
  boost::asio::mutable_buffer cb(buffer,
                                 sizeof(buffer));
  SECTION("All") {
    prefix_buffer_sequence pbs(cb,
                               sizeof(buffer));
    auto iter = boost::asio::buffer_sequence_begin(pbs);
    REQUIRE(iter != boost::asio::buffer_sequence_end(pbs));
    std::optional<boost::asio::mutable_buffer> cb(*iter);
    CHECK(cb->size() == sizeof(buffer));
    CHECK(cb->data() == buffer);
    ++iter;
    CHECK(iter == boost::asio::buffer_sequence_end(pbs));
    --iter;
    REQUIRE(iter != boost::asio::buffer_sequence_end(pbs));
    cb.emplace(*iter);
    CHECK(cb->size() == sizeof(buffer));
    CHECK(cb->data() == buffer);
    CHECK(boost::asio::buffer_size(pbs) == sizeof(buffer));
    char* begin = buffer;
    char* end = begin + sizeof(buffer);
    CHECK(std::equal(begin,
                     end,
                     boost::asio::buffers_begin(pbs),
                     boost::asio::buffers_end(pbs)));
    CHECK(std::equal(std::reverse_iterator(end),
                     std::reverse_iterator(begin),
                     std::reverse_iterator(boost::asio::buffers_end(pbs)),
                     std::reverse_iterator(boost::asio::buffers_begin(pbs))));
  }
  SECTION("Non-empty prefix") {
    prefix_buffer_sequence pbs(cb,
                               sizeof(buffer) - 1);
    auto iter = boost::asio::buffer_sequence_begin(pbs);
    REQUIRE(iter != boost::asio::buffer_sequence_end(pbs));
    std::optional<boost::asio::mutable_buffer> cb(*iter);
    CHECK(cb->size() == (sizeof(buffer) - 1));
    CHECK(cb->data() == buffer);
    ++iter;
    CHECK(iter == boost::asio::buffer_sequence_end(pbs));
    --iter;
    REQUIRE(iter != boost::asio::buffer_sequence_end(pbs));
    cb.emplace(*iter);
    CHECK(cb->size() == (sizeof(buffer) - 1));
    CHECK(cb->data() == buffer);
    CHECK(boost::asio::buffer_size(pbs) == (sizeof(buffer) - 1));
    char* begin = buffer;
    char* end = begin + sizeof(buffer) - 1;
    CHECK(std::equal(begin,
                     end,
                     boost::asio::buffers_begin(pbs),
                     boost::asio::buffers_end(pbs)));
    CHECK(std::equal(std::reverse_iterator(end),
                     std::reverse_iterator(begin),
                     std::reverse_iterator(boost::asio::buffers_end(pbs)),
                     std::reverse_iterator(boost::asio::buffers_begin(pbs))));
  }
  SECTION("Empty prefix") {
    prefix_buffer_sequence pbs(cb,
                               0);
    auto iter = boost::asio::buffer_sequence_begin(pbs);
    CHECK(iter == boost::asio::buffer_sequence_end(pbs));
    CHECK(boost::asio::buffer_size(pbs) == 0);
    char* begin = buffer;
    char* end = begin;
    CHECK(std::equal(begin,
                     end,
                     boost::asio::buffers_begin(pbs),
                     boost::asio::buffers_end(pbs)));
    CHECK(std::equal(std::reverse_iterator(end),
                     std::reverse_iterator(begin),
                     std::reverse_iterator(boost::asio::buffers_end(pbs)),
                     std::reverse_iterator(boost::asio::buffers_begin(pbs))));
  }
}

}
}
