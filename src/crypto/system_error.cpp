#include <mcpp/crypto/system_error.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <system_error>
#include <mcpp/checked.hpp>
#include <openssl/err.h>

namespace mcpp::crypto {

std::error_code make_error_code(unsigned long e) noexcept {
  auto i = mcpp::checked_cast<int>(e);
  if (!i) {
    return make_error_code(std::errc::value_too_large);
  }
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/OpenSSL";
    }
    virtual std::string message(int code) const override {
      unsigned long native(code);
      std::string retr;
      retr.resize(256);
      ::ERR_error_string_n(native,
                           retr.data(),
                           retr.size());
      assert(std::find(retr.begin(),
                       retr.end(),
                       '\0') != retr.end());
      retr.resize(std::strlen(retr.data()));
      return retr;
    }
  } category;
  return std::error_code(*i,
                         category);
}

std::error_code get_error_code() noexcept {
  auto e = ::ERR_get_error();
  while (::ERR_get_error());
  return crypto::make_error_code(e);
}

}
