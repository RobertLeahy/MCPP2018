#include <mcpp/nbt/sax_parse.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <ios>
#include <iterator>
#include <system_error>
#include <variant>
#include <vector>
#include <boost/asio/error.hpp>
#include <boost/core/noncopyable.hpp>
#include <mcpp/nbt/test/sax_observer.hpp>
#include <mcpp/system_error.hpp>
#include <zlib.h>

#include <catch2/catch.hpp>

namespace mcpp::nbt::tests {
namespace {

template<typename Event>
class event_type_predicate {
public:
  bool operator()(const test::sax_observer::event_type& event) const noexcept {
    return std::holds_alternative<Event>(event);
  }
};

TEST_CASE("sax_parse",
          "[mcpp][nbt][sax][sax_parse]")
{
  std::vector<std::byte> in;
  test::sax_observer observer;
  std::error_code ec;
  SECTION("test.nbt") {
    in.push_back(std::byte{0x0a});
    in.push_back(std::byte{0x00});
    in.push_back(std::byte{0x0b});
    in.push_back(std::byte{0x68});
    in.push_back(std::byte{0x65});
    in.push_back(std::byte{0x6c});
    in.push_back(std::byte{0x6c});
    in.push_back(std::byte{0x6f});
    in.push_back(std::byte{0x20});
    in.push_back(std::byte{0x77});
    in.push_back(std::byte{0x6f});
    in.push_back(std::byte{0x72});
    in.push_back(std::byte{0x6c});
    in.push_back(std::byte{0x64});
    in.push_back(std::byte{0x08});
    in.push_back(std::byte{0x00});
    in.push_back(std::byte{0x04});
    in.push_back(std::byte{0x6e});
    in.push_back(std::byte{0x61});
    in.push_back(std::byte{0x6d});
    in.push_back(std::byte{0x65});
    in.push_back(std::byte{0x00});
    in.push_back(std::byte{0x09});
    in.push_back(std::byte{0x42});
    in.push_back(std::byte{0x61});
    in.push_back(std::byte{0x6e});
    in.push_back(std::byte{0x61});
    in.push_back(std::byte{0x6e});
    in.push_back(std::byte{0x72});
    in.push_back(std::byte{0x61});
    in.push_back(std::byte{0x6d});
    in.push_back(std::byte{0x61});
    in.push_back(std::byte{0x00});
    SECTION("Complete") {
      auto result = sax_parse(in.begin(),
                              in.end(),
                              observer,
                              ec);
      CHECK(result == in.end());
      REQUIRE_FALSE(ec);
      auto iter = observer.events.begin();
      REQUIRE_FALSE(iter == observer.events.end());
      auto begin = std::get<test::sax_observer::begin_event>(*iter);
      CHECK(begin.begin == in.begin());
      CHECK(begin.end == in.end());
      ++iter;
      REQUIRE_FALSE(iter == observer.events.end());
      auto tag = std::get<test::sax_observer::tag_event>(*iter);
      CHECK(tag.tag == std::byte{10});
      CHECK(tag.begin == in.begin());
      CHECK(tag.end == (in.begin() + 1));
      ++iter;
      REQUIRE_FALSE(iter == observer.events.end());
      auto name = std::get<test::sax_observer::name_event>(*iter);
      CHECK(name.name == "hello world");
      CHECK(name.begin == (in.begin() + 1));
      CHECK(name.end == (in.begin() + 1 + 13));
      ++iter;
      REQUIRE_FALSE(iter == observer.events.end());
      auto begin_compound = std::get<test::sax_observer::begin_compound_event>(*iter);
      CHECK(begin_compound.where == (in.begin() + 1 + 13));
      ++iter;
      REQUIRE_FALSE(iter == observer.events.end());
      tag = std::get<test::sax_observer::tag_event>(*iter);
      CHECK(tag.tag == std::byte{8});
      CHECK(tag.begin == (in.begin() + 1 + 13));
      CHECK(tag.end == (in.begin() + 1 + 13 + 1));
      ++iter;
      REQUIRE_FALSE(iter == observer.events.end());
      name = std::get<test::sax_observer::name_event>(*iter);
      CHECK(name.name == "name");
      CHECK(name.begin == (in.begin() + 1 + 13 + 1));
      CHECK(name.end == (in.begin() + 1 + 13 + 1 + 6));
      ++iter;
      REQUIRE_FALSE(iter == observer.events.end());
      auto string = std::get<test::sax_observer::string_event>(*iter);
      CHECK(string.value == "Bananrama");
      CHECK(string.begin == (in.begin() + 1 + 13 + 1 + 6));
      CHECK(string.end == (in.begin() + 1 + 13 + 1 + 6 + 11));
      ++iter;
      REQUIRE_FALSE(iter == observer.events.end());
      tag = std::get<test::sax_observer::tag_event>(*iter);
      CHECK(tag.tag == std::byte{0});
      CHECK(tag.begin == (in.end() - 1));
      CHECK(tag.end == in.end());
      ++iter;
      REQUIRE_FALSE(iter == observer.events.end());
      auto end_compound = std::get<test::sax_observer::end_compound_event>(*iter);
      CHECK(end_compound.where == in.end());
      ++iter;
      REQUIRE_FALSE(iter == observer.events.end());
      auto end = std::get<test::sax_observer::end_event>(*iter);
      CHECK(end.where == in.end());
      ++iter;
      CHECK(iter == observer.events.end());
    }
    SECTION("Incomplete") {
      while (!in.empty()) {
        observer.events.clear();
        in.pop_back();
        INFO(in.size() << " bytes");
        auto result = sax_parse(in.begin(),
                                in.end(),
                                observer,
                                ec);
        CHECK(result == in.end());
        CHECK(ec);
        CHECK(ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
        REQUIRE_FALSE(observer.events.empty());
        INFO(observer.events.back().index());
        auto error = std::get<test::sax_observer::error_event>(observer.events.back());
        CHECK(error.where == in.end());
        CHECK(error.ec);
        CHECK(error.ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
      }
    }
  }
  SECTION("bigtest.nbt") {
    std::filebuf buf;
    auto ptr = buf.open(MCPP_BIGTEST_PATH,
                        std::ios_base::in | std::ios_base::binary);
    REQUIRE(ptr);
    std::vector<char> compressed;
    std::copy(std::istreambuf_iterator<char>(&buf),
              std::istreambuf_iterator<char>(),
              std::back_inserter(compressed));
    REQUIRE(compressed.size() == 507);
    {
      ::z_stream stream;
      stream.next_in = reinterpret_cast<::Bytef*>(compressed.data());
      stream.avail_in = compressed.size();
      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = nullptr;
      auto result = ::inflateInit2(&stream,
                                   15 + 32);
      REQUIRE(result == Z_OK);
      class guard : private boost::noncopyable {
      public:
        explicit guard(::z_stream* ptr) noexcept
          : ptr_(ptr)
        {
          assert(ptr_);
        }
        ~guard() noexcept {
          assert(ptr_);
          auto result = ::inflateEnd(ptr_);
          assert(result == Z_OK);
          (void)result;
        }
      private:
        ::z_stream* ptr_;
      };
      guard g(&stream);
      in.resize(1544);
      stream.next_out = reinterpret_cast<::Bytef*>(in.data());
      stream.avail_out = in.size();
      result = inflate(&stream,
                       true);
      REQUIRE(result == Z_STREAM_END);
      REQUIRE(stream.avail_out == 0);
    }
    SECTION("Complete") {
      auto result = sax_parse(in.begin(),
                              in.end(),
                              observer,
                              ec);
      CHECK(result == in.end());
      REQUIRE_FALSE(ec);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::begin_event>()) == 1);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::error_event>()) == 0);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::end_event>()) == 1);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::tag_event>()) == 30);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::name_event>()) == 22);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::begin_compound_event>()) == 6);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::end_compound_event>()) == 6);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::string_event>()) == 5);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::byte_event>()) == 1001);
      CHECK(std::count_if(observer.events.begin(),
                          observer.events.end(),
                          event_type_predicate<test::sax_observer::int_event>()) == 1);
    }
    SECTION("Incomplete") {
      while (!in.empty()) {
        observer.events.clear();
        in.pop_back();
        INFO(in.size() << " bytes");
        auto result = sax_parse(in.begin(),
                                in.end(),
                                observer,
                                ec);
        CHECK(result == in.end());
        CHECK(ec);
        CHECK(ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
        REQUIRE_FALSE(observer.events.empty());
        INFO(observer.events.back().index());
        auto error = std::get<test::sax_observer::error_event>(observer.events.back());
        CHECK(error.where == in.end());
        CHECK(error.ec);
        CHECK(error.ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
      }
    }
  }
}

}
}
