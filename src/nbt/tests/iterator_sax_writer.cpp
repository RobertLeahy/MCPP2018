#include <mcpp/nbt/iterator_sax_writer.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>
#include <boost/core/noncopyable.hpp>
#include <mcpp/nbt/sax_parse.hpp>
#include <zlib.h>

#include <catch2/catch.hpp>

namespace mcpp::nbt::tests {
namespace {

TEST_CASE("iterator_sax_writer",
          "[mcpp][nbt][sax][iterator_sax_writer]")
{
  std::vector<std::byte> vec;
  iterator_sax_writer writer(std::back_inserter(vec));
  SECTION("test.nbt") {
    std::vector<std::byte> expected;
    expected.push_back(std::byte{0x0a});
    expected.push_back(std::byte{0x00});
    expected.push_back(std::byte{0x0b});
    expected.push_back(std::byte{0x68});
    expected.push_back(std::byte{0x65});
    expected.push_back(std::byte{0x6c});
    expected.push_back(std::byte{0x6c});
    expected.push_back(std::byte{0x6f});
    expected.push_back(std::byte{0x20});
    expected.push_back(std::byte{0x77});
    expected.push_back(std::byte{0x6f});
    expected.push_back(std::byte{0x72});
    expected.push_back(std::byte{0x6c});
    expected.push_back(std::byte{0x64});
    expected.push_back(std::byte{0x08});
    expected.push_back(std::byte{0x00});
    expected.push_back(std::byte{0x04});
    expected.push_back(std::byte{0x6e});
    expected.push_back(std::byte{0x61});
    expected.push_back(std::byte{0x6d});
    expected.push_back(std::byte{0x65});
    expected.push_back(std::byte{0x00});
    expected.push_back(std::byte{0x09});
    expected.push_back(std::byte{0x42});
    expected.push_back(std::byte{0x61});
    expected.push_back(std::byte{0x6e});
    expected.push_back(std::byte{0x61});
    expected.push_back(std::byte{0x6e});
    expected.push_back(std::byte{0x72});
    expected.push_back(std::byte{0x61});
    expected.push_back(std::byte{0x6d});
    expected.push_back(std::byte{0x61});
    expected.push_back(std::byte{0x00});
    writer.begin();
    writer.tag(std::byte{10});
    writer.name("hello world");
    writer.begin_compound();
    writer.tag(std::byte{8});
    writer.name("name");
    writer.value("Bananrama");
    writer.tag(std::byte{0});
    writer.end_compound();
    writer.end();
    CHECK(std::equal(vec.begin(),
                     vec.end(),
                     expected.begin(),
                     expected.end()));
  }
  SECTION("String too long") {
    std::string str;
    std::size_t i(std::numeric_limits<std::uint16_t>::max());
    ++i;
    str.resize(i,
               'a');
    CHECK_THROWS_AS(writer.value(str),
                    std::overflow_error);
  }
  SECTION("::iterator") {
    writer.tag(std::byte{10});
    *writer.iterator() = std::byte{0};
    REQUIRE(vec.size() == 2);
    CHECK(vec[0] == std::byte{10});
    CHECK(vec[1] == std::byte{0});
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
    std::vector<std::byte> in;
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
    std::error_code ec;
    auto result = sax_parse(in.begin(),
                            in.end(),
                            writer,
                            ec);
    REQUIRE(result == in.end());
    REQUIRE_FALSE(ec);
    CHECK(std::equal(in.begin(),
                     in.end(),
                     vec.begin(),
                     vec.end()));
  }
}

}
}
