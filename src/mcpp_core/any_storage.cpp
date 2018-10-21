#include <mcpp/any_storage.hpp>

#include <cassert>
#include <cstdlib>
#include <new>

namespace mcpp {

any_storage::any_storage() noexcept
  : ptr_ (nullptr),
    size_(0),
    dtor_(nullptr)
{}

any_storage::any_storage(any_storage&& other) noexcept
  : ptr_ (other.ptr_),
    size_(other.size_),
    dtor_(other.dtor_)
{
  other.ptr_ = nullptr;
  other.size_ = 0;
  other.dtor_ = nullptr;
}

any_storage& any_storage::operator=(any_storage&& rhs) noexcept {
  reset();
  assert(!dtor_);
  free();
  assert(!ptr_);
  assert(!size_);
  ptr_ = rhs.ptr_;
  rhs.ptr_ = nullptr;
  size_ = rhs.size_;
  rhs.size_ = 0;
  dtor_ = rhs.dtor_;
  rhs.dtor_ = nullptr;
  return *this;
}

any_storage::~any_storage() noexcept {
  reset();
  assert(!dtor_);
  free();
}

auto any_storage::capacity() const noexcept -> size_type {
  return size_;
}

void any_storage::reset() noexcept {
  if (!dtor_) {
    return;
  }
  assert(ptr_);
  assert(size_);
  dtor_(ptr_);
  dtor_ = nullptr;
}

void any_storage::reserve(size_type new_cap) {
  reset();
  assert(!dtor_);
  if (new_cap < size_) {
    return;
  }
  void* tmp = std::realloc(ptr_,
                           new_cap);
  if (!tmp) {
    throw std::bad_alloc();
  }
  ptr_ = tmp;
  size_ = new_cap;
}

any_storage::operator bool() const noexcept {
  return has_value();
}

bool any_storage::has_value() const noexcept {
  return bool(dtor_);
}

void any_storage::free() noexcept {
  std::free(ptr_);
  ptr_ = nullptr;
  size_ = 0;
}

}
