#include "./Page.h"
#include <fstream>

namespace simplevm {

  Page::Page(fstream& swap_file, pno_t virtual_pno):swap_file_(swap_file) { 
    virtual_pno_ = virtual_pno; 
    dirty_ = false;
    bytes_ = new uint8_t[PAGE_SIZE];  
    special_ = false;

    pno_t page = virtual_pno_*PAGE_SIZE;
    swap_file_.seekg(page);
    swap_file_.read(reinterpret_cast<char*>(bytes_), PAGE_SIZE);
  }

  Page::Page(const Page& other):swap_file_(other.swap_file_) {
    virtual_pno_ = other.virtual_pno_; 
    dirty_ = other.dirty_;
    bytes_ = new uint8_t[PAGE_SIZE];  
    for (unsigned i = 0; i < PAGE_SIZE; i ++) {
      bytes_[i] = other.bytes_[i];
    }
  }

  Page::~Page(){ 
    if (dirty_ == true) {
      flush();
    }
    delete[] bytes_;
  }

  Page& Page::operator=(const Page& rhs) {
    if (*this==rhs) {
      return *this;
    }
    virtual_pno_ = rhs.virtual_pno_; 
    dirty_ = rhs.dirty_;
    bytes_ = new uint8_t[PAGE_SIZE];  
    for (unsigned i = 0; i < PAGE_SIZE; i ++) {
      bytes_[i] = rhs.bytes_[i];
    }
    return *this;
  }

  bool Page::operator==(const Page& rhs) {
    if (
      virtual_pno_ == rhs.virtual_pno_ &&
      dirty_ == rhs.dirty_ &&
      bytes_ == rhs.bytes_
    ) {
      return true;
    } 
    return false;
  }

  bool Page:: operator<(const Page& rhs){
    if (virtual_pno_ > rhs.virtual_pno_) {
      return true;
    }
    return false;
  }
  
  pno_t Page:: pno() {
    return virtual_pno_;
  }

  void Page:: setSpecial() {
    special_ = true;
  }

  bool Page:: viewSpecial() {
    return special_;
  }

  bool Page:: dirty() {
    return dirty_;
  }

  void Page:: flush() {
    if (dirty()){
      pno_t page = virtual_pno_*PAGE_SIZE;
      // std::cout<<"* flushing page num = " << page << "\n"<<std::endl;
      swap_file_.seekg(page);
      swap_file_.write(reinterpret_cast<char*>(bytes_), PAGE_SIZE);
      swap_file_.flush();
      dirty_ = false;
    }
  }
}
