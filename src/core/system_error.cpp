#include <mcpp/system_error.hpp>

#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <system_error>
#include <boost/asio/error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/unordered_set.hpp>

namespace mcpp {

namespace {

const boost::system::error_category& to_boost_error_category(const std::error_category&) noexcept;

const std::error_category& to_error_category(const boost::system::error_category&) noexcept;

class wrapper : public boost::system::error_category {
public:
  explicit wrapper(const std::error_category& c) noexcept
    : c_(c)
  {}
  virtual const char* name() const noexcept override {
    return c_.name();
  }
  virtual std::string message(int code) const override {
    return c_.message(code);
  }
  virtual boost::system::error_condition default_error_condition(int code) const noexcept override {
    auto ec = c_.default_error_condition(code);
    return boost::system::error_condition(ec.value(),
                                          to_boost_error_category(ec.category()));
  }
  virtual bool equivalent(int code,
                          const boost::system::error_condition& condition) const noexcept override
  {
    std::error_condition ec(condition.value(),
                            mcpp::to_error_category(condition.category()));
    return c_.equivalent(code,
                         ec);
  }
  virtual bool equivalent(const boost::system::error_code& code,
                          int condition) const noexcept override
  {
    return c_.equivalent(mcpp::to_error_code(code),
                         condition);
  }
  const std::error_category& get() const noexcept {
    return c_;
  }
private:
  const std::error_category& c_;
};

class hash {
public:
  std::size_t operator()(const std::error_category& category) const noexcept {
    std::hash<const std::error_category*> hasher;
    return hasher(&category);
  }
  std::size_t operator()(const wrapper& w) const noexcept {
    return (*this)(w.get());
  }
};

class equal_to {
public:
  template<typename T,
           typename U>
  bool operator()(const T& a,
                  const U& b) const noexcept
  {
    return &unwrap(a) == &unwrap(b);
  }
private:
  static const std::error_category& unwrap(const wrapper& w) noexcept {
    return w.get();
  }
  static const std::error_category& unwrap(const std::error_category& category) noexcept {
    return category;
  }
};

const boost::system::error_category& to_boost_error_category(const std::error_category& category) noexcept {
  using map_type = boost::unordered_set<wrapper,
                                        hash,
                                        equal_to>;
  static map_type map;
  static std::mutex m;
  {
    std::lock_guard l(m);
    auto iter = map.find(category,
                         map.hash_function(),
                         map.key_eq());
    if (iter != map.end()) {
      return *iter;
    }
    try {
      auto pair = map.emplace(category);
      assert(pair.second);
      return *pair.first;
    } catch (...) {}
  }
  static const class : public boost::system::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/To Boost Error Category";
    }
    virtual std::string message(int code) const override {
      if (!code) {
        return "Success";
      }
      return "Could not allocate memory for a boost::system::error_category to wrap input std::error_category";
    }
    virtual boost::system::error_condition default_error_condition(int code) const noexcept override {
      if (!code) {
        return boost::system::error_condition();
      }
      return make_error_code(boost::system::errc::not_enough_memory).default_error_condition();
    }
  } fallback;
  return fallback;
}

const std::error_category& to_error_category(const boost::system::error_category& category) noexcept {
  auto ptr = dynamic_cast<const wrapper*>(&category);
  if (ptr) {
    return ptr->get();
  }
  return category;
}

}

std::error_code to_error_code(boost::system::error_code ec) noexcept {
  return std::error_code(ec.value(),
                         mcpp::to_error_category(ec.category()));
}

std::error_code to_error_code(std::error_code ec) noexcept {
  return ec;
}

bool is_eof(std::error_code ec) noexcept {
  return ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition();
}

bool is_eof(boost::system::error_code ec) noexcept {
  std::error_code sec(ec.value(),
                      mcpp::to_error_category(ec.category()));
  return sec.default_error_condition() == make_error_code(boost::asio::error::eof).default_error_condition();
}

boost::system::error_code to_boost_error_code(boost::system::error_code ec) noexcept {
  return ec;
}


boost::system::error_code to_boost_error_code(std::error_code ec) noexcept {
  return boost::system::error_code(ec.value(),
                                   mcpp::to_boost_error_category(ec.category()));
}

}
