/**
 *  \file
 */

#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <system_error>
#include <vector>

namespace mcpp::nbt {

/**
 *  A model of `SaxObserver` which receives events
 *  from the NBT SAX parser and writes them to a
 *  `std::ostream` for debugging purposes.
 */
class ostream_sax_observer {
public:
  /**
   *  Creates an ostream_sax_observer.
   *
   *  \param [in] os
   *    The `std::ostream` to write to.
   */
  explicit ostream_sax_observer(std::ostream& os) noexcept;
#ifndef MCPP_DOXYGEN_RUNNING
  using allocator_type = std::allocator<void>;
  template<typename T>
  using vector_type = std::vector<T>;
  using string_type = std::string;
  allocator_type get_allocator() const noexcept;
  template<typename InputIterator>
  std::error_code begin(InputIterator,
                        InputIterator)
  {
    *os_ << "BEGIN\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code end(InputIterator) {
    *os_ << "END" << std::endl;
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code tag(std::byte tag,
                      InputIterator,
                      InputIterator)
  {
    *os_ << "TAG: " << std::to_integer<unsigned int>(tag) << '\n';
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code name(string_type name,
                       InputIterator,
                       InputIterator)
  {
    *os_ << "NAME: " << name << '\n';
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code begin_compound(InputIterator) {
    *os_ << "BEGIN COMPOUND\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code end_compound(InputIterator) {
    *os_ << "END COMPOUND\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code value(std::int8_t value,
                        InputIterator,
                        InputIterator)
  {
    *os_ << int(value) << '\n';
    return std::error_code();
  }
  template<typename Value,
           typename InputIterator>
  std::error_code value(Value value,
                        InputIterator,
                        InputIterator)
  {
    *os_ << value << '\n';
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code begin_list(InputIterator) {
    *os_ << "BEGIN LIST\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code end_list(InputIterator) {
    *os_ << "END LIST\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code length(std::int32_t length,
                         InputIterator,
                         InputIterator)
  {
    *os_ << "LENGTH: " << length << '\n';
    return std::error_code();
  }
  template<typename InputIterator>
  void error(std::error_code ec,
             InputIterator)
  {
    *os_ << "ERROR: " << ec.message() << std::endl;
  }
  template<typename InputIterator>
  std::error_code begin_byte_array(InputIterator) {
    *os_ << "BEGIN BYTE ARRAY\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code end_byte_array(InputIterator) {
    *os_ << "END BYTE ARRAY\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code begin_int_array(InputIterator) {
    *os_ << "BEGIN INT ARRAY\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code end_int_array(InputIterator) {
    *os_ << "END INT ARRAY\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code begin_long_array(InputIterator) {
    *os_ << "BEGIN LONG ARRAY\n";
    return std::error_code();
  }
  template<typename InputIterator>
  std::error_code end_long_array(InputIterator) {
    *os_ << "END LONG ARRAY\n";
    return std::error_code();
  }
#endif
private:
  std::ostream* os_;
};

}
