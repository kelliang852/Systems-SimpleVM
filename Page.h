#ifndef PAGE_H_
#define PAGE_H_

#include <cstdint>
#include <fstream>

using std::fstream;

namespace simplevm {

// defines the type pno_t, which is the type
// that represents a page number
typedef uint32_t pno_t;

///////////////////////////////////////////////////////////////////////////////
// A Page is a class that represents a page of memory
// in our simple virtual memory model.
// If a page object exists, then we say that the page is loaded
// into physical memory. When the page object doesn't exist, then its
// data is stored in the swap_file. When we load in a page to
// "physical memory", we are creating the page and we read the page's data
// from the swap file. A page's data in the swap file starts at
// virtual_pno * Page::PAGE_SIZE
//
// This Class manages a page's worth of data
// Users can access or store data, sa well as flush the data in the
// page to the specified swap file. A swap file is where exceess virtual
// memory is stored when it can't fit in physical memory.
///////////////////////////////////////////////////////////////////////////////
class Page {
 public:
  // Constructs a new Page object associated
  // with a swap_file and a virtual page number.
  // The swap file is where we will load in the page
  // contents and flush the page contents. The virtual
  // page number decides where in that file we read
  // and write this page.
  // Passing in an invalid page number is undefined behaviour


    // where you store the data inside the page
    // initialise on the heap, should be of page size
    // dynamically allocated, direct copy content from swap files to the array


  //
  // Arguments:
  //  - swap_file the swap_file associated with the page
  //  - the virtual page number of our new page
  Page(fstream& swap_file, pno_t virtual_pno);

  // Constructs a new Page object that is a copy of
  // another page object. Both pages will have
  // the same page number and swap_file, but should
  // have independent copies of the page data.
  // 
  // Misc: this means that there could be issues with
  // having the original and copy page having differnt
  // data. This cctor should only really be used
  // in the context of managing pages with something
  // like STL, where the original page used for the cctor
  // will be discarded. In real C++, we would want to
  // use move semantics here.
  //
  // Arguements:
  //   - other: the page we are copying
  Page(const Page& other);

  // Destructor for the page object
  // Cleans up any dynamically allocated data or
  // otherwise allocated resources AND should flush
  // its contents if the page is dirty at time of
  // destruction.
  ~Page();

  // Set the current Page object so that is a copy of
  // another page object. Both pages will have
  // the same page number and swap_file, but should
  // have independent copies of the page data. 
  // 
  // Misc: this means that there could be issues with
  // having the original and copy page having differnt
  // data. This op= should only really be used
  // in the context of managing pages with something
  // like STL, where the original page used for the cctor
  // will be discarded. In real C++, we would want to
  // use move semantics here.
  //
  // You can assume each page has the same swap_file.
  //
  // Arguements:
  //   - rhs: the page we are copying
  Page& operator=(const Page& rhs);

  // This function is not required, but you may add it
  // if it is needed for some of the STL containers
  // you use in PageTable
  //
  // Determines if this page should go before another page if they
  // were in sorted order.
  //
  // Arguments:
  //   - rhs: the Page we are comparing this to
  //
  // Returns: true iff this page would show up before the other
  // page in sorted order. False otherwise.
  bool operator<(const Page& rhs);

  // This function allows users to read various data types
  // from the page. Trying to read a non-primitive type or use
  // a virtual address that doesn't map to this page results
  // in undefined behaviour. You can also assume that
  // anything being read fits in on the page we are reading 
  // is not partially on another page.
  // If you are familiar with endianness, it shouldn't be
  // considered for this function.
  //
  // Arguments:
  //   - virtual_address: a virtual address that maps somewhere
  //     into the page, where we will read data of type T
  //
  // Returns:
  //   - the data of type T that was read from the page
  template <typename T>
  T access(uint32_t virtual_address);

  // This function allows users to write various data types
  // to the page. Trying to write a non-primitive type or use
  // a virtual address that doesn't map to this page results
  // in undefined behaviour. You can also assume that
  // anything being written fits on the current page
  // is not partially on another page.
  // If you are familiar with endianness, it shouldn't be
  // considered for this function.
  //
  // Arguments:
  //   - virtual_address: a virtual address that maps somewhere
  //     into the page, where we will read data of type T
  //   - to_write: the data of type T to write to the page
  //
  // Returns: nothing
  template <typename T>
  void store(uint32_t virtual_address, const T& to_write);

  // Returns the virtual page number of this page
  //
  // Arguments: None
  //
  // Returns: this page's virtual page number
  pno_t pno();

  // Returns whether or not a page is dirty
  // A page is "dirty" if someone has written to the data managed
  // by the page since the last time the page was flush()'d.
  //
  // Arguments: None
  //
  // Returns: Whether this page is dirty or not
  bool dirty();

  // Flushes the page to the swap file if it is dirty.
  // Flushing a page to the swap file involves writing
  // the page at the the spot correspoding to its page number
  // in the swap_file. For a description of what it means
  // for a page to be dirty, see the dirty() member function.
  // The page should not be written if it is not dirty.
  // 
  // Arguments: None
  // 
  // Returns: Nothing
  void flush();
  
  // The amount of memory a page represents
  static constexpr size_t PAGE_SIZE = 4096U;

  void setSpecial();
  bool viewSpecial();

 private:
  // The file we will be reading/writing to
  // Note how this is a reference
  fstream& swap_file_;

  // the virtual page number
  pno_t virtual_pno_;

  // The bytes of the page. One byte is 8 bits
  // so we use 8-bit unsigned integers.
  // You can also assume that a 'char' is one byte big
  uint8_t *bytes_;

  // Whether the page is dirty or not
  bool dirty_;

  bool operator==(const Page& rhs);
  
  bool special_; 
  
};

}

// since we have template code
#include "./PageTemplates.cc"

#endif  // PAGE_H_

