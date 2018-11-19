#include <mcpp/lowest_layer.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <boost/asio/basic_socket.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

static_assert(!has_lowest_layer_v<int>);
static_assert(has_lowest_layer_v<boost::asio::ip::tcp::socket>);
static_assert(std::is_same_v<lowest_layer_t<boost::asio::ip::tcp::socket>,
                             boost::asio::ip::tcp::socket::lowest_layer_type>);
static_assert(!std::is_same_v<boost::asio::ip::tcp::socket,
                              boost::asio::ip::tcp::socket::lowest_layer_type>);
static_assert(std::is_same_v<int,
                             lowest_layer_t<int>>);

TEST_CASE("get_lowest_layer (no lowest layer)",
          "[mcpp][lowest_layer]")
{
  int i = 5;
  auto p = std::addressof(get_lowest_layer(i));
  CHECK(p == &i);
  const int c = 5;
  auto q = std::addressof(get_lowest_layer(c));
  CHECK(q == &c);
}

class mock {
public:
  using lowest_layer_type = int;
  explicit mock(int& i) noexcept
    : i_(i)
  {}
  lowest_layer_type& lowest_layer() noexcept {
    return i_;
  }
  const lowest_layer_type& lowest_layer() const noexcept {
    return i_;
  }
private:
  int& i_;
};

TEST_CASE("get_lowest_layer (lowest layer)",
          "[mcpp][lowest_layer]")
{
  int i = 5;
  mock m(i);
  CHECK(get_lowest_layer(m) == 5);
  CHECK(std::addressof(get_lowest_layer(m)) == &i);
  CHECK(get_lowest_layer(std::as_const(m)) == 5);
  CHECK(std::addressof(get_lowest_layer(std::as_const(m))) == &i);
}

}
}
