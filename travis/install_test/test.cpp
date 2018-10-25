#include <cassert>
#include <cstdlib>
#include <mcpp/any_storage.hpp>

int main() {
  mcpp::any_storage storage;
  int& i = storage.emplace<int>(EXIT_SUCCESS);
  assert(!i);
  return i;
}
