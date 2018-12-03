#include <mcpp/hex_dump.hpp>

#include <cstddef>
#include <sstream>
#include <vector>
#include <boost/asio/buffer.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

TEST_CASE("hex_dump",
          "[mcpp][core][hex_dump]")
{
  hex_dump::settings_type settings;
  std::ostringstream ss;
  hex_dump dumper(settings,
                  ss);
  std::vector<std::byte> buffer;
  SECTION("Immediately done") {
    dumper.done();
    auto str = ss.str();
    CHECK(str.empty());
  }
  SECTION("Empty & done") {
    boost::asio::const_buffer cb;
    dumper(cb);
    dumper.done();
    auto str = ss.str();
    CHECK(str.empty());
  }
  SECTION("One & done") {
    buffer.push_back(std::byte{0});
    buffer.push_back(std::byte{1});
    buffer.push_back(std::byte{2});
    buffer.push_back(std::byte{3});
    dumper(boost::asio::buffer(buffer));
    dumper.done();
    auto str = ss.str();
    CHECK(str == "00 01 02 03                                      ....");
    //            00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
  }
  SECTION("Two & done (across multiple lines)") {
    buffer.push_back(std::byte{0});
    buffer.push_back(std::byte{1});
    buffer.push_back(std::byte{2});
    buffer.push_back(std::byte{3});
    dumper(boost::asio::buffer(buffer));
    buffer.clear();
    buffer.push_back(std::byte{4});
    buffer.push_back(std::byte{5});
    buffer.push_back(std::byte{6});
    buffer.push_back(std::byte{7});
    buffer.push_back(std::byte{8});
    buffer.push_back(std::byte{9});
    buffer.push_back(std::byte{10});
    buffer.push_back(std::byte{11});
    buffer.push_back(std::byte{12});
    buffer.push_back(std::byte{13});
    buffer.push_back(std::byte{14});
    buffer.push_back(std::byte{15});
    buffer.push_back(std::byte{16});
    dumper(boost::asio::buffer(buffer));
    dumper.done();
    auto str = ss.str();
    CHECK(str == "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f  ................\n"
                 "10                                               .");
  }
  SECTION("Output after") {
    buffer.push_back(std::byte{0});
    buffer.push_back(std::byte{1});
    buffer.push_back(std::byte{2});
    buffer.push_back(std::byte{3});
    dumper(boost::asio::buffer(buffer));
    dumper.done();
    ss << '\n' << 16;
    auto str = ss.str();
    CHECK(str == "00 01 02 03                                      ....\n"
                 "16");
  }
  SECTION("Printable") {
    buffer.push_back(std::byte{0});
    buffer.push_back(std::byte{1});
    buffer.push_back(std::byte{2});
    buffer.push_back(std::byte{3});
    dumper(boost::asio::buffer(buffer));
    buffer.clear();
    buffer.push_back(std::byte{u8'A'});
    buffer.push_back(std::byte{u8'B'});
    buffer.push_back(std::byte{6});
    buffer.push_back(std::byte{7});
    buffer.push_back(std::byte{8});
    buffer.push_back(std::byte{9});
    buffer.push_back(std::byte{10});
    buffer.push_back(std::byte{11});
    buffer.push_back(std::byte{12});
    buffer.push_back(std::byte{13});
    buffer.push_back(std::byte{14});
    buffer.push_back(std::byte{15});
    buffer.push_back(std::byte{16});
    dumper(boost::asio::buffer(buffer));
    dumper.done();
    auto str = ss.str();
    CHECK(str == "00 01 02 03 41 42 06 07 08 09 0a 0b 0c 0d 0e 0f  ....AB..........\n"
                 "10                                               .");
  }
}

}
}
