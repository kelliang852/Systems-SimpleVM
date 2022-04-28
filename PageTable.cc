#include "./PageTable.h"
#include <fstream> 
#include <map>
#include <queue>

namespace simplevm {

  int currnum = 0;

  PageTable:: PageTable(std::string swap_file_name, size_t page_capacity){
    swap_file_.open (swap_file_name, std::fstream::in | std::fstream::out);
    capacity_ = page_capacity;
  }

  PageTable:: ~PageTable(){
    flush_all_pages();
    for (auto& i :stl) {
      delete i.second;
    }
    swap_file_.close();
  }

  size_t PageTable:: capacity(){
    return capacity_;
  }

  size_t PageTable:: loaded_pages(){
    return stl.size();
  }

  bool PageTable:: page_available(pno_t virtual_pno){
    for (auto &i : stl) {
      Page* curr = i.second;
      if (curr->pno() == virtual_pno) {
        return true;
      }
    }
    return false;
  }

  void PageTable:: discard_page(pno_t virtual_pno){
    for (auto &i : stl) {
      Page* curr = i.second;
      if (curr->pno() == virtual_pno) {
        if (curr->dirty()) {
          curr->flush();
        }
        stl.erase (i.first);
        delete curr; 
        return;
      }
    }
  }

  void PageTable:: flush_all_pages(){
    for (auto &i : stl) {
      Page* curr = i.second;
      if (curr->dirty()) {
        curr->flush();
      }
    }
  }

  void PageTable:: flush_page(pno_t virtual_pno){
    for (auto &i : stl) {
      Page* curr = i.second;
      if (curr->pno() == virtual_pno) {
        curr->flush();
        return;
      }
    }
  }

  Page& PageTable:: get_page(uint32_t virtual_address){
    pno_t pnum = virtual_address>>12;
    if (page_available(pnum)) { 
      int ind = 0;
      for (auto &i : stl) {
        Page* curr = i.second;
        if (curr->pno() == pnum) {
          ind = i.first;
        }
      } 
      Page* fromtbl = stl.at(ind);
      Page& ref = *fromtbl;
      stl.erase(ind); 
      currnum++;
      stl.insert(std::pair<int, Page*>(currnum, &ref));
      return ref;
    } 

    if (!(stl.size() < capacity_)) {
      evict_page();
    } 

    Page* newpage = new Page(swap_file_, pnum);
    currnum++; 
    stl.insert(std::pair<int, Page*>(currnum, newpage)); 
    return *newpage;
  }

  void PageTable:: evict_page() {
    if (loaded_pages() == 0) {
      return;
    }
    int currMin = stl.begin()->first; 
    for (auto &i : stl) {
      if (currMin > i.first) {
        currMin = i.first;
      }
    }
    Page* evict = stl.at(currMin);
    if (evict->dirty()) {
      evict->flush();
    }
    delete evict;
    stl.erase(currMin);
  }
}
