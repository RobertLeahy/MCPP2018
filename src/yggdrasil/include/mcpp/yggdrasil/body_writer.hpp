/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/optional.hpp>
#include "error.hpp"
#include <mcpp/system_error.hpp>
#include <rapidjson/memorybuffer.h>
#include <rapidjson/writer.h>

namespace mcpp::yggdrasil {

namespace detail {

enum class body_writer_error {
  success = 0,
  rapidjson_failed
};

std::error_code make_error_code(body_writer_error) noexcept;

template<typename Allocator>
class body_writer_base {
public:
  using const_buffers_type = boost::asio::const_buffer;
  using allocator_type = Allocator;
  explicit body_writer_base(const Allocator&) {}
  void init(boost::beast::error_code& ec) noexcept {
    ec.clear();
    buffer_.Clear();
  }
protected:
  template<typename T>
  boost::optional<std::pair<const_buffers_type,
                            bool>> get(const T& message,
                                       std::error_code& ec)
  {
    ec.clear();
    ::rapidjson::Writer<::rapidjson::MemoryBuffer> w(buffer_);
    if (!to_json(message,
                 w))
    {
      ec = make_error_code(body_writer_error::rapidjson_failed);
      return boost::none;
    }
    const_buffers_type retr(buffer_.GetBuffer(),
                            buffer_.GetSize());
    return std::pair(retr,
                     false);
  }
private:
  ::rapidjson::MemoryBuffer buffer_;
};

}

/**
 *  A model of the `BodyWriter` concept which:
 *
 *  1. Reduces the body to a series of SAX events
 *     via RapidJSON
 *  2. Collects the results of those SAX events in
 *     an in memory buffer
 *  3. Releases that buffer via `ConstBufferSequence`
 *     to be written via Boost.Beast.
 *
 *  \tparam isRequest
 *    `true` if a request should be parsed, `false`
 *    otherwise.
 *  \tparam T
 *    The type of object to serialize.
 *  \tparam Fields
 *    A model of the `Fields` concept. Defaults to
 *    `boost::beast::http::fields`.
 *  \tparam Allocator
 *    A model of the `Allocator` concept. Defaults
 *    to `std::allocator<void>`.
 */
template<bool isRequest,
         typename T,
         typename Fields = boost::beast::http::fields,
         typename Allocator = std::allocator<void>>
class body_writer
#ifndef MCPP_DOXYGEN_RUNNING
: public detail::body_writer_base<Allocator>
#endif
{
private:
  using base = detail::body_writer_base<Allocator>;
public:
  /**
   *  Exposes the `isRequest` template parameter as
   *  a static member variable.
   */
  static constexpr bool is_request = isRequest;
  /**
   *  If the `isRequest` template parameter is `true`
   *  then a type alias for `T`, otherwise a type
   *  alias for a `std::variant` over `T` and
   *  \ref error.
   */
  using value_type =
#ifdef MCPP_DOXYGEN_RUNNING
  SEE_DESCRIPTION
#else
  T
#endif
  ;
  /**
   *  A type alias for the `Fields` template parameter.
   */
  using fields_type = Fields;
  /**
   *  A type alias for the `Allocator` template parameter.
   */
  using allocator_type = Allocator;
  /**
   *  A type alias for the result of instantiating
   *  `boost::beast::http::header` with \ref is_request
   *  and \ref fields_type.
   *
   *  Expected as the first argument to the constructor.
   */
  using header_type = boost::beast::http::header<is_request,
                                                 fields_type>;
  /**
   *  Creates a body_writer.
   *
   *  The lifetime guarantees of all arguments except `alloc`
   *  are as described in the description of the
   *  `BodyWriter` concept.
   *
   *  \param [in] h
   *    The object which models the \ref header_type "header".
   *  \param [in] v
   *    The object which models the \ref value_type "body".
   *  \param [in] alloc
   *    The `Allocator` to use. Defaults to a default constructed
   *    \ref allocator_type.
   */
  body_writer(const header_type& h,
              const value_type& v,
              const allocator_type& alloc = allocator_type())
    : base(alloc),
      v_  (&v)
  {}
#ifndef MCPP_DOXYGEN_RUNNING
  auto get(boost::beast::error_code& ec) {
    assert(v_);
    ec.clear();
    std::error_code sec;
    auto retr = base::get(*v_,
                          sec);
    if (sec) {
      ec = mcpp::to_boost_error_code(sec);
    }
    return retr;
  }
  template<typename U>
  body_writer(const U&,
              const value_type&);
#endif
private:
  const value_type* v_;
};

#ifndef MCPP_DOXYGEN_RUNNING
template<typename T,
         typename Fields,
         typename Allocator>
class body_writer<false,
                  T,
                  Fields,
                  Allocator>
#ifndef MCPP_DOXYGEN_RUNNING
: public detail::body_writer_base<Allocator>
#endif
{
private:
  using base = detail::body_writer_base<Allocator>;
public:
  static constexpr bool is_request = false;
  using value_type = std::variant<T,
                                  error>;
  using fields_type = Fields;
  using allocator_type = Allocator;
  using header_type = boost::beast::http::header<is_request,
                                                 fields_type>;
  body_writer(const header_type& h,
              const value_type& v,
              const allocator_type& alloc = allocator_type())
    : base(alloc),
      v_  (&v)
  {}
  auto get(boost::beast::error_code& ec) {
    assert(v_);
    ec.clear();
    std::error_code sec;
    auto visitor = [&, this](auto&& obj) {
      return base::get(obj,
                       sec);
    };
    auto retr = std::visit(visitor,
                           *v_);
    if (sec) {
      ec = mcpp::to_boost_error_code(sec);
    }
    return retr;
  }
  template<typename U>
  body_writer(const U&,
              const value_type&);
private:
  const value_type* v_;
};
#endif

}
