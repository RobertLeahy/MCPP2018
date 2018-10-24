#include <mcpp/test/allocator.hpp>

namespace mcpp::test {

allocator_state::allocator_state() noexcept
  : allocate  (0),
    deallocate(0)
{}

allocator<void>::allocator(state& state) noexcept
  : state_(&state)
{}

}
