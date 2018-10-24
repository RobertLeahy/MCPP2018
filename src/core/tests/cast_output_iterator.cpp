#include <mcpp/cast_output_iterator.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <vector>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

TEST_CASE("cast_output_iterator",
          "[mcpp][cast_output_iterator][iterator][core]")
{
  std::vector<char> in;
  in.push_back('f');
  in.push_back('o');
  in.push_back('o');
  std::array<std::byte,
             4> arr;
  std::fill(arr.begin(),
            arr.end(),
            std::byte{0});
  auto iter = std::copy(in.begin(),
                        in.end(),
                        make_cast_output_iterator<std::byte>(arr.begin())).base();
  CHECK((iter - arr.begin()) == 3);
  CHECK(arr[0] == std::byte{'f'});
  CHECK(arr[1] == std::byte{'o'});
  CHECK(arr[2] == std::byte{'o'});
  CHECK(arr[3] == std::byte{0});
}

}
}
