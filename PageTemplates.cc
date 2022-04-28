#include<iostream>
#include <stdio.h>
#include <string.h>

namespace simplevm { 
  
  template <typename T>
  T Page::access(uint32_t virtual_address) {
    pno_t address = virtual_address & 0x00000FFF;
    uint8_t* tmp = &bytes_[address]; 
    T* res = reinterpret_cast<T*>(tmp);
    return *res;
  }

  template <typename T>
  void Page::store(uint32_t virtual_address, const T& to_write) { 
    pno_t address = virtual_address & 0x00000FFF;
    void* dest = bytes_ + address;
    memcpy(dest, &to_write, sizeof(to_write));
    dirty_ = true;
  }
}
