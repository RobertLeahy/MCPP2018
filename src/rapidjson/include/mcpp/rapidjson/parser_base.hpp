/**
 *  \file
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <system_error>

namespace mcpp::rapidjson {

/** 
 *  A convenience base class for implementing
 *  RapidJSON SAX handlers. Implements all required
 *  functions and if any of those functions are called
 *  returns `false` and sets an internal `std::error_code`.
 */
class parser_base {
public:
#ifndef MCPP_DOXYGEN_RUNNING
  bool Null() noexcept;
  bool Bool(bool) noexcept;
  bool Int(int) noexcept;
  bool Uint(unsigned) noexcept;
  bool Int64(std::int64_t) noexcept;
  bool Uint64(std::uint64_t) noexcept;
  bool Double(double) noexcept;
  bool String(const char*,
              std::size_t,
              bool) noexcept;
  bool StartObject() noexcept;
  bool Key(const char*,
           std::size_t,
           bool) noexcept;
  bool EndObject(std::size_t) noexcept;
  bool StartArray() noexcept;
  bool EndArray(std::size_t) noexcept;
  bool RawNumber(const char*,
                 std::size_t,
                 bool) noexcept;
#endif
  /**
   *  Returns the stored `std::error_code`.
   *
   *  \return
   *    The stored `std::error_code`. Will be
   *    falsey if no error has been set.
   */
  std::error_code error_code() const noexcept;
  /**
   *  Clears the stored `std::error_code` so
   *  that this object can be reused.
   */
  void clear() noexcept;
protected:
  /**
   *  Replaces the stored `std::error_code`.
   *
   *  \param [in] ec
   *    The `std::error_code` to store.
   */
  void error_code(std::error_code ec) noexcept;
private:
  std::error_code ec_;
};

}
