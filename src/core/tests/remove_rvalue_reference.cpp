#include <mcpp/remove_rvalue_reference.hpp>

#include <type_traits>

namespace mcpp::tests {
namespace {

static_assert(std::is_same_v<const int,
                             remove_rvalue_reference_t<const int>>);
static_assert(std::is_same_v<const int&,
                             remove_rvalue_reference_t<const int&>>);
static_assert(std::is_same_v<const int,
                             remove_rvalue_reference_t<const int&&>>);
static_assert(std::is_same_v<int,
                             remove_rvalue_reference_t<int>>);
static_assert(std::is_same_v<int&,
                             remove_rvalue_reference_t<int&>>);
static_assert(std::is_same_v<int,
                             remove_rvalue_reference_t<int&&>>);

}
}
