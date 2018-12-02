/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <optional>
#include <system_error>
#include <type_traits>
#include <variant>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include "error.hpp"
#include <mcpp/system_error.hpp>
#include <rapidjson/memorystream.h>
#include <rapidjson/reader.h>

namespace mcpp::yggdrasil {

namespace detail {

enum class body_reader_error {
  success = 0,
  rapidjson_failed,
  no_body
};

std::error_code make_error_code(body_reader_error) noexcept;

template<typename Allocator = std::allocator<void>>
class body_reader_base {
public:
  using allocator_type = Allocator;
  explicit body_reader_base(const Allocator& alloc = Allocator())
    : buffer_(alloc)
  {}
  template<typename T>
  void init(const T& t,
            boost::beast::error_code& ec)
  {
    ec.clear();
    buffer_.clear();
    if (t) {
      buffer_.reserve(*t);
    }
  }
  template<typename ConstBufferSequence>
  std::size_t put(ConstBufferSequence cb,
                  boost::beast::error_code& ec)
  {
    ec.clear();
    using buffers_iterator_type = boost::asio::buffers_iterator<ConstBufferSequence,
                                                                std::byte>;
    buffer_.insert(buffer_.end(),
                   buffers_iterator_type::begin(cb),
                   buffers_iterator_type::end(cb));
    return boost::asio::buffer_size(cb);
  }
protected:
  template<typename Parser>
  std::error_code finish(Parser& parser) {
    parser.clear();
    ::rapidjson::MemoryStream ms(reinterpret_cast<const ::rapidjson::MemoryStream::Ch*>(buffer_.data()),
                                 buffer_.size());
    ::rapidjson::Reader r;
    auto result = r.Parse(ms,
                          parser);
    if (result) {
      assert(!parser.error_code());
      return std::error_code();
    }
    auto retr = parser.error_code();
    if (retr) {
      return retr;
    }
    return make_error_code(body_reader_error::rapidjson_failed);
  }
private:
  using internal_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<std::byte>;
  using buffer_type = std::vector<std::byte,
                                  internal_allocator_type>;
  buffer_type buffer_;
};

}

/**
 *  A model of the `BodyReader` concept which:
 *
 *  1. Collects the body of an HTTP request or
 *     response (depending on the `isRequest`
 *     template parameter)
 *  2. Parses the body as JSON using RapidJSON
 *     emitting all SAX events generated therefrom
 *     to an instance of `Parser`
 *  3. Checks the result of that parse and reports
 *     it as a `boost::beast::error_code`
 *
 *  \tparam isRequest
 *    `true` if a request should be parsed, `false`
 *    otherwise.
 *  \tparam T
 *    The type of object to parse.
 *  \tparam Parser
 *    The parser to which SAX events shall be emitted.
 *    Must bo constructible with a single argument of
 *    type `T&` and must model the RapidJSON `Reader`
 *    concept in addition to having:
 *    - Nullary `clear`
 *    - Nullary `done` returning `bool`
 *  \tparam Fields
 *    A model of the `Fields` concept. Defaults to
 *    `boost::beast::http::fields`.
 *  \tparam Allocator
 *    A model of the `Allocator` concept. Defaults
 *    to `std::allocator<void>`.
 */
template<bool isRequest,
         typename T,
         typename Parser,
         typename Fields = boost::beast::http::fields,
         typename Allocator = std::allocator<void>>
class body_reader
#ifndef MCPP_DOXYGEN_RUNNING
: public detail::body_reader_base<Allocator>
#endif
{
private:
  using base = detail::body_reader_base<Allocator>;
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
   *  A type alias for the `Parser` template parameter.
   */
  using parser_type = Parser;
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
   *  Creates a body_reader.
   *
   *  The lifetime guarantees of all arguments except `alloc`
   *  are as described in the description of the
   *  `BodyReader` concept.
   *
   *  \param [in] h
   *    The object which models the \ref header_type "header".
   *  \param [in] v
   *    The object which models the \ref value_type "body".
   *  \param [in] alloc
   *    The `Allocator` to use. Defaults to a default constructed
   *    \ref allocator_type.
   */
  body_reader(const header_type& h,
              value_type& v,
              const allocator_type& alloc = allocator_type())
    : base(alloc),
      v_  (&v)
  {}
#ifndef MCPP_DOXYGEN_RUNNING
  template<typename U>
  void init(const U& u,
            boost::beast::error_code& ec)
  {
    assert(v_);
    ec.clear();
    base::init(u,
               ec);
    if (ec) {
      return;
    }
    parser_.emplace(*v_);
  }
  void finish(boost::beast::error_code& ec) {
    if (parser_) {
      ec = mcpp::to_boost_error_code(base::finish(*parser_));
      return;
    }
    ec = mcpp::to_boost_error_code(make_error_code(detail::body_reader_error::no_body));
  }
  template<typename U>
  body_reader(const U&,
              const value_type&);
#endif
private:
  value_type*                v_;
  std::optional<parser_type> parser_;
};

#ifndef MCPP_DOXYGEN_RUNNING
template<typename T,
         typename Parser,
         typename Fields,
         typename Allocator>
class body_reader<false,
                  T,
                  Parser,
                  Fields,
                  Allocator> : public detail::body_reader_base<Allocator>
{
private:
  using base = detail::body_reader_base<Allocator>;
public:
  static constexpr bool is_request = false;
  using value_type = std::variant<T,
                                  error>;
  using parser_type = Parser;
  using fields_type = Fields;
  using allocator_type = Allocator;
  using header_type = boost::beast::http::header<is_request,
                                                 fields_type>;
  body_reader(const header_type& h,
              value_type& v,
              const allocator_type& alloc = allocator_type())
    : base(alloc),
      h_  (&h),
      v_  (&v)
  {}
  template<typename U>
  void init(const U& u,
            boost::beast::error_code& ec)
  {
    assert(h_);
    assert(v_);
    ec.clear();
    base::init(u,
               ec);
    if (ec) {
      return;
    }
    auto code = h_->result_int();
    if (code == 200) {
      init_impl<T,
                parser_type>();
      return;
    }
    init_impl<error,
              error_parser>();
  }
  void finish(boost::beast::error_code& ec) {
    //  The explicit capture of this is necessary
    //  otherwise GCC 8.2.0 encounters an ICE
    auto visitor = [&, this](auto&& parser) {
      if constexpr (std::is_same_v<std::decay_t<decltype(parser)>,
                                   std::monostate>)
      {
        ec = mcpp::to_boost_error_code(make_error_code(detail::body_reader_error::no_body));
      } else {
        ec = mcpp::to_boost_error_code(base::finish(parser));
      }
    };
    std::visit(visitor,
               parser_);
  }
  template<typename U>
  body_reader(const U&,
              const value_type&);
private:
  template<typename U,
           typename V>
  void init_impl() {
    auto&& obj = v_->template emplace<U>();
    parser_.template emplace<V>(obj);
  }
  using internal_parser_type = std::variant<std::monostate,
                                            parser_type,
                                            error_parser>;
  const header_type*   h_;
  value_type*          v_;
  internal_parser_type parser_;
};
#endif

}
