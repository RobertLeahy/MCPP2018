#include <mcpp/nbt/ostream_sax_observer.hpp>

namespace mcpp::nbt {

ostream_sax_observer::ostream_sax_observer(std::ostream& os) noexcept
  : os_(&os)
{}

auto ostream_sax_observer::get_allocator() const noexcept -> allocator_type {
  return allocator_type();
}

}
