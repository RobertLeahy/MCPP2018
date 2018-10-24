#include <mcpp/async_result.hpp>

#include <type_traits>
#include <boost/asio/use_future.hpp>
#include <boost/system/error_code.hpp>

namespace mcpp::test {
namespace {

static_assert(std::is_same_v<int,
                             completion_handler_t<int,
                                                  void()>>);
static_assert(std::is_same_v<int,
                             completion_handler_t<int&&,
                                                  void()>>);
static_assert(!std::is_same_v<boost::asio::use_future_t<>,
                              completion_handler_t<boost::asio::use_future_t<>,
                                                   void(boost::system::error_code)>>);

}
}
