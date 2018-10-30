#include <mcpp/zlib/system_error.hpp>

#include <string>
#include <system_error>
#include <boost/asio/error.hpp>
#include <errno.h>
#include <mcpp/system_error.hpp>
#include <zlib.h>

namespace mcpp::zlib {

std::error_code make_error_code(int code) noexcept {
  static_assert(!Z_OK);
  if (code == Z_ERRNO) {
      return std::error_code(errno,
                             std::system_category());
  }
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/ZLIB";
    }
    virtual std::string message(int code) const override {
      switch (code) {
      case Z_OK:
        return "Success";
      case Z_STREAM_END:
        return "End of stream";
      case Z_NEED_DICT:
        return "Need dictionary";
      case Z_ERRNO:
        return "Consult errno";
      case Z_STREAM_ERROR:
        return "Stream error";
      case Z_DATA_ERROR:
        return "Data error";
      case Z_MEM_ERROR:
        return "Memory error";
      case Z_BUF_ERROR:
        return "Buffer error";
      case Z_VERSION_ERROR:
        return "Version error";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (code) {
      case Z_OK:
        return std::error_condition();
      case Z_STREAM_END:
        return to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition();
      case Z_NEED_DICT:
      case Z_ERRNO:
      case Z_STREAM_ERROR:
      case Z_DATA_ERROR:
      case Z_BUF_ERROR:
      case Z_VERSION_ERROR:
      default:
        break;
      case Z_MEM_ERROR:
        return make_error_code(std::errc::not_enough_memory).default_error_condition();
      }
      return std::error_condition(code,
                                  *this);
    }
  } category;
  return std::error_code(code,
                         category);
}

}
