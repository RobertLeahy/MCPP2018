#include <mcpp/remove_optional.hpp>

#include <optional>
#include <type_traits>

namespace mcpp::tests {
namespace {

template<typename Expected,
         typename T>
constexpr bool test_v = std::is_same_v<Expected,
                                       remove_optional_t<T>>;

static_assert(test_v<int,
                     std::optional<int>>);
static_assert(test_v<int,
                     int>);
static_assert(test_v<int,
                     const std::optional<int>>);
static_assert(test_v<int,
                     volatile std::optional<int>>);
static_assert(test_v<int,
                     const volatile std::optional<int>>);

}
}
