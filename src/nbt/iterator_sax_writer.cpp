#include <mcpp/nbt/iterator_sax_writer.hpp>

#include <limits>
#include <sstream>
#include <stdexcept>

namespace mcpp::nbt {

namespace detail {

void iterator_sax_writer_string_length_error(std::size_t size) {
  std::ostringstream ss;
  ss << "String has length " << size << " which is longer than maximum representable NBT string length " << std::numeric_limits<std::uint16_t>::max();
  throw std::overflow_error(ss.str());
}

}

}
