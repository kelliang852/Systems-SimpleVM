

#ifndef PAGE_TABLE_H_
#define PAGE_TABLE_H_

#include <fstream>
#include <cstdint>
#include <map>

#include "./Page.h"

using std::fstream;

namespace simplevm {

///////////////////////////////////////////////////////////////////////////////
// A PageTable manages a processes memeory for our simplified
// virtual memory model. This involves managing a swap_file
// which is where pages of data are stored when they aren't loaded
// into physical memory. For our software model, we will say a page
// is in "physical memory" if it is loaded into our memory space
// (e.g. it is on the heap). Pages that aren't loaded in will have
// their contents stored in the swap_file and will not have an
// associated Page object (see Page.h). Our page table can only have
// so many pages stored in memory at one time, which is specified
// on PageTable Creation. We implement an LRU page replacement
// policy to decide which pages to evict if we need to load a new page
// and we already have reached our capacity on the numberof pages we can
// hold.
//
// Users can get a page from the cache, flush pages to the swap_file, 
// request any page is evicted, and specifically ask for a page to be evicted.
///////////////////////////////////////////////////////////////////////////////
class PageTable {
 public:
  // Constructs a new page table with the specified
  // swap file and the specified page capacity, which is
  // the number of pages that can be held in memory 
  // at one time. There cannot be more than page_capacity
  // number of pages loaded in at a time.
  //
  // Arguments:
  //   - swap_file_name: the name of the swap_file
  //   - page_capacity: the maximum number of pages that can be held
  //     in memory at one time.
  PageTable(std::string swap_file_name, size_t page_capacity);

  // Destructs the page table, freeing any allocated resources
  // and flushing any pages currently loaded into memory that
  // are dirty
  ~PageTable();

  // Given a virtual address, gets the associated
  // page for that virtual address. This page will
  // be "loaded" into physical memory by the time it
  // is returned.
  //
  // There are three possiblities when a page is requested:
  // 1. The page is currently in the "loaded" and in the cache.
  //    In this case, a reference to the page is returned and
  //    and the page is marked as most recently used in the cache
  // 2. The page is not currently "loaded", and the PageTable
  //    has not reached its page capacity:
  //    In this case, the page is loaded from the swap file and added
  //    to the cache as the most recently used page.
  // 3. The page is not currently "loaded", and the PageTable
  //    is at page capacity:
  //    The least recently used page in the cache is evicted from the
  //    cache. Afterwards the requested page is loaded from the swap file
  //    and added to the cache as the most recently used page.
  //
  // NOTE: What decides how recntly used a page was used is entirely
  // decided by how recntly it was returned by a call to get_page.
  //
  // Arguments:
  //   - virtual_address: A virtual address that is associated
  //     with a requested page. The virutal address is represented
  //     as a unsigned 32 bit integer. NOTE: a virtual address
  //     is NOT the same as a page number. Multiple virtual addresses
  //     could be associated with the same page number.
  //
  // Returns:
  //   - the requested page, which is loaded into the cache and
  //     marked as the most recently used page
  Page& get_page(uint32_t virtual_address);

  // Returns the page capacity of the page table
  //
  // Arguments: None
  //
  // Returns: the page capacity of the page table
  size_t capacity();

  // Returns the number of pages currently loaded into "physical memory"
  //
  // Arguments: None
  //
  // Returns: the number of pages currently loaded into "physical memory"
  size_t loaded_pages();
  
  // Checks to see if the specified page is loaded into memory
  //
  // Arguments: The virtual page number of the page to check for
  //
  // Returns: True iff the page is loaded into memory, false otherwise
  bool page_available(pno_t virtual_pno);

  // Makes sure that all currently loaded pages are flushed
  // meaning tha the page contents are updated on the swap file.
  // This should not affect how recently used each page is and all pages
  // will remain loaded into memory after this operation is performed.
  //
  // Arguments: None
  //
  // Returns: Nothing
  void flush_all_pages();

  // Flushes the specified page to the swap file.
  // This should not affect how recently used each page is and all pages
  // will remain loaded into memory after this operation is performed.
  //
  // Arguments: the virtual page number of the page to flush
  //
  // Returns: Nothing
  void flush_page(pno_t virtual_pno);
  
  // Discards the specified page from the PageTable.
  // If the page is dirty, then it is flushed before it is discarded.
  // If the page is not in the table, then nothing happens.
  //
  // Arguments: the virtual page number of the page to discard.
  //
  // Returns: Nothing
  void discard_page(pno_t virtual_pno);

  // Evicts a page from the PageTable. The page evicted
  // should be the least recntly used page in the cache.
  // If the evicted page is dirty, then it is flushed before it is evicted.
  // If there are no pages in the cache, then do nothing.
  //
  // Arguments: None
  //
  // Returns: Nothing
  void evict_page();

 private:
  // The swap file where pages are stored
  fstream swap_file_;

  // The number of pages that can be stored
  // in the PageTable at one time.
  size_t capacity_;

  std::map<int, Page*> stl;

  // TODO: add fields

};

}


#endif  // PAGE_TABLE_H_
