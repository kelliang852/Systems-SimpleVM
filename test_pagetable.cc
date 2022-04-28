/*
 * Copyright ©2022 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 595 for use solely during Spring Semester 2022 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include "gtest/gtest.h"
#include "./test_suite.h"

#include "./Page.h"
#include "./PageTable.h"

#include <iostream>

using std::fstream;
using std::ios_base;
using simplevm::Page;
using simplevm::PageTable;

namespace hw4 {

class Test_PageTable : public ::testing::Test {
 protected:
  // Code here will be called before all tests execute (ie, before
  // any TEST_F).
  virtual void SetUp() {
    fstream s(kSimpleFileName, ios_base::in);
    fstream copy;
    copy.open(kSimpleCopyFileName, ios_base::in | ios_base::out | ios_base::trunc);
    char bytes[Page::PAGE_SIZE];
    s.read(bytes, Page::PAGE_SIZE);
    while (!s.eof()) {
      copy.write(bytes, Page::PAGE_SIZE);
      s.read(bytes, Page::PAGE_SIZE);
    }

    copy.close();

    fstream c(kComplexFileName, ios_base::in);
    copy.open(kComplexCopyFileName, ios_base::in | ios_base::out | ios_base::trunc);
    c.read(bytes, Page::PAGE_SIZE);
    while (!c.eof()) {
      copy.write(bytes, Page::PAGE_SIZE);
      c.read(bytes, Page::PAGE_SIZE);
    }
  }

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    // Do nothing
  }

 public:
  // These values are used as constants for the PageTable tests.
  // They cannot be const, as stored value pointers are non-const.
  static constexpr const char* kSimpleFileName = "./test_files/simple.bytes";
  static constexpr const char* kSimpleCopyFileName = "./test_files/simple_copy.bytes";
  static constexpr const char* kComplexFileName = "./test_files/complex.bytes";
  static constexpr const char* kComplexCopyFileName = "./test_files/complex_copy.bytes";
  
};  // class Test_PageTable

TEST_F(Test_PageTable, ctor) {
  HW4Environment::OpenTestCase();
  PageTable *pt = new PageTable(kSimpleCopyFileName, 2);
  for(uint32_t i = 0; i < 32U; i++) {
    ASSERT_FALSE(pt->page_available(i));
  }
  ASSERT_EQ(2U, pt->capacity());
  ASSERT_EQ(0U, pt->loaded_pages()); 
  delete pt;
  HW4Environment::AddPoints(5);
}

TEST_F(Test_PageTable, get_page_simple) {
  HW4Environment::OpenTestCase();
  PageTable *pt = new PageTable(kSimpleCopyFileName, 2);
  ASSERT_EQ(0U, pt->get_page(0).pno());
  ASSERT_EQ(1U, pt->loaded_pages()); 
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));

  ASSERT_EQ(1U, pt->get_page(Page::PAGE_SIZE).pno());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_EQ(2U, pt->capacity());
  ASSERT_EQ(2U, pt->loaded_pages()); 

  HW4Environment::AddPoints(5);

  ASSERT_EQ(3U, pt->get_page(3 * Page::PAGE_SIZE + 3872).pno());
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_EQ(2U, pt->capacity());
  ASSERT_EQ(2U, pt->loaded_pages()); 

  ASSERT_EQ(1U, pt->get_page(6612).pno());
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_EQ(2U, pt->capacity());
  ASSERT_EQ(2U, pt->loaded_pages()); 

  ASSERT_EQ(2U, pt->get_page(10223).pno());
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_EQ(2U, pt->capacity());
  ASSERT_EQ(2U, pt->loaded_pages()); 

  delete pt;

  HW4Environment::AddPoints(10);
}

TEST_F(Test_PageTable, discard_page) {
  HW4Environment::OpenTestCase();
  PageTable *pt = new PageTable(kSimpleCopyFileName, 3U);
  ASSERT_EQ(0U, pt->get_page(0U).pno());
  ASSERT_EQ(1U, pt->loaded_pages()); 
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));

  // discarded pages not loaded should do nothing
  pt->discard_page(1U);
  pt->discard_page(2U);
  pt->discard_page(3U);
  ASSERT_EQ(1U, pt->loaded_pages()); 
  ASSERT_EQ(3U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));

  HW4Environment::AddPoints(5);

  // discard a page that is not the LRU
  ASSERT_EQ(0U, pt->get_page(0U).pno());
  ASSERT_EQ(1U, pt->get_page(1U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(2U, pt->loaded_pages()); 
  ASSERT_EQ(3U, pt->capacity());

  pt->discard_page(1U);
  
  ASSERT_EQ(1U, pt->loaded_pages()); 
  ASSERT_EQ(3U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));

  // Discard a page that is the LRU
  ASSERT_EQ(0U, pt->get_page(0U).pno());
  ASSERT_EQ(2U, pt->get_page(2U * Page::PAGE_SIZE + 13).pno());
  ASSERT_EQ(1U, pt->get_page(2U * Page::PAGE_SIZE - 1).pno());

  pt->discard_page(0U);

  ASSERT_EQ(2U, pt->loaded_pages()); 
  ASSERT_EQ(3U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));

  // Clear the remaining pages
  pt->discard_page(2U);
  pt->discard_page(1U);
  ASSERT_EQ(0U, pt->loaded_pages()); 
  ASSERT_EQ(3U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));

  delete pt;

  HW4Environment::AddPoints(10);
}

TEST_F(Test_PageTable, evict_page) {
  HW4Environment::OpenTestCase();
  PageTable *pt = new PageTable(kComplexCopyFileName, 4U);
  ASSERT_EQ(0U, pt->get_page(0U).pno());
  ASSERT_EQ(1U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());

  ASSERT_EQ(1U, pt->get_page(1U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(2U, pt->get_page(2U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(0U, pt->get_page(0U).pno());
  ASSERT_EQ(3U, pt->get_page(3U * Page::PAGE_SIZE).pno());
  pt->evict_page();
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(4U, pt->get_page(4U * Page::PAGE_SIZE).pno());
  pt->evict_page();
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(5U, pt->get_page(5U * Page::PAGE_SIZE).pno());
  pt->evict_page();
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_TRUE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(0U, pt->get_page(0U * Page::PAGE_SIZE).pno());
  pt->evict_page();
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_TRUE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(1U, pt->get_page(1U * Page::PAGE_SIZE).pno());
  pt->evict_page();
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(4U));
  ASSERT_TRUE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(2U, pt->get_page(2U * Page::PAGE_SIZE).pno());
  pt->evict_page();
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(3U, pt->get_page(3U * Page::PAGE_SIZE).pno());
  pt->evict_page();
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(4U, pt->get_page(4U * Page::PAGE_SIZE).pno());
  pt->evict_page();
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(7U, pt->get_page(7U * Page::PAGE_SIZE).pno());
  pt->evict_page();
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_TRUE(pt->page_available(7U));
  
  pt->evict_page();
  ASSERT_EQ(2U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_TRUE(pt->page_available(7U));

  pt->evict_page();
  ASSERT_EQ(1U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_TRUE(pt->page_available(7U));

  pt->evict_page();
  ASSERT_EQ(0U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  delete pt;

  HW4Environment::AddPoints(20);
}

TEST_F(Test_PageTable, flush) {
  HW4Environment::OpenTestCase();
  PageTable *pt = new PageTable(kComplexCopyFileName, 6U);
  uint32_t original5 = pt->get_page(5 * Page::PAGE_SIZE + 182).access<uint32_t>(5 * Page::PAGE_SIZE + 12);
  std::cout<<"original5 = "<<original5<<"\n"<<std::endl;

  uint32_t original1 = pt->get_page(2 * Page::PAGE_SIZE - 37).access<uint32_t>(Page::PAGE_SIZE + 1);
  std::cout<<"original1 = "<<original1<<"\n"<<std::endl;

  uint32_t original3 = pt->get_page(3 * Page::PAGE_SIZE + 2).access<uint32_t>(3 * Page::PAGE_SIZE + 1984);
  std::cout<<"original3 = "<<original3<<"\n"<<std::endl;
  
  uint32_t original4 = pt->get_page(4 * Page::PAGE_SIZE + 90).access<uint32_t>(4 * Page::PAGE_SIZE + 2814);
  std::cout<<"original4 = "<<original4<<"\n"<<std::endl;

  ASSERT_EQ(4U, pt->loaded_pages()); 
  // std::cout <<"not flushed (1) \n" <<std::endl;

  ASSERT_EQ(6U, pt->capacity());
  // std::cout <<"not flushed (2) \n" <<std::endl;

  pt->get_page(5 * Page::PAGE_SIZE + 182).store<uint32_t>(5 * Page::PAGE_SIZE + 12, original5 % 2);
  // std::cout <<"not flushed (3) \n" <<std::endl;

  pt->get_page(2 * Page::PAGE_SIZE - 37).store<uint32_t>(Page::PAGE_SIZE + 1, original1 + 17);
  // std::cout <<"not flushed" <<std::endl;

  pt->flush_page(5U);
  // std::cout <<"done flushed\n" <<std::endl;

  // check that 5 was flushed
  uint32_t in_file;
  fstream reader(kComplexCopyFileName, ios_base::in);
  reader.seekg(5*Page::PAGE_SIZE + 12, ios_base::beg);
  reader.read(reinterpret_cast<char*>(&in_file), sizeof(in_file));
  ASSERT_EQ(original5 % 2, in_file);
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(6U, pt->capacity());
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_TRUE(pt->page_available(5U));
  
  // and that 1 was not flushed 
  reader.seekg(Page::PAGE_SIZE + 1, ios_base::beg);
  reader.read(reinterpret_cast<char*>(&in_file), sizeof(in_file));
  ASSERT_EQ(original1, in_file);
  HW4Environment::AddPoints(10);

  // make sure that fluh_all_pages flushes every page
  pt->get_page(3 * Page::PAGE_SIZE).store<uint32_t>(3 * Page::PAGE_SIZE + 1984, original3 + 505);
  pt->flush_all_pages();
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(6U, pt->capacity());
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_TRUE(pt->page_available(5U));

  // check that 1 and 3 were flushed
  reader.seekg(Page::PAGE_SIZE + 1, ios_base::beg);
  reader.read(reinterpret_cast<char*>(&in_file), sizeof(in_file));
  ASSERT_EQ(original1 + 17, in_file);

  reader.seekg(3 * Page::PAGE_SIZE + 1984, ios_base::beg);
  reader.read(reinterpret_cast<char*>(&in_file), sizeof(in_file));
  ASSERT_EQ(original3 + 505, in_file);
  HW4Environment::AddPoints(5);

  // write to page 4, then discard the page, checking to make sure discard flushed the page
  pt->get_page(4 * Page::PAGE_SIZE).store<uint32_t>(4 * Page::PAGE_SIZE + 2814, original4 + 21);
  pt->discard_page(4U);
  ASSERT_EQ(3U, pt->loaded_pages()); 
  ASSERT_EQ(6U, pt->capacity());
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(5U));

  reader.seekg(4 * Page::PAGE_SIZE + 2814, ios_base::beg);
  reader.read(reinterpret_cast<char*>(&in_file), sizeof(in_file));
  ASSERT_EQ(original4 + 21, in_file);

  delete pt;

  HW4Environment::AddPoints(5);
}

TEST_F(Test_PageTable, get_page_complex) {
  HW4Environment::OpenTestCase();
  PageTable *pt = new PageTable(kComplexCopyFileName, 4U);
  // std::cout<<"loaded pages= " <<pt->loaded_pages()<<std::endl;
  ASSERT_EQ(0U, pt->get_page(0U).pno());
  ASSERT_EQ(1U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());

  ASSERT_EQ(1U, pt->get_page(1U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(2U, pt->get_page(2U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(0U, pt->get_page(0U).pno());
  ASSERT_EQ(3U, pt->get_page(3U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(4U, pt->get_page(4U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(5U, pt->get_page(5U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_TRUE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(0U, pt->get_page(0U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_TRUE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(1U, pt->get_page(1U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_FALSE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_TRUE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(2U, pt->get_page(2U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_FALSE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(4U));
  ASSERT_TRUE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(3U, pt->get_page(3U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_TRUE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_FALSE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(4U, pt->get_page(4U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_TRUE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_FALSE(pt->page_available(7U));

  ASSERT_EQ(7U, pt->get_page(7U * Page::PAGE_SIZE).pno());
  ASSERT_EQ(4U, pt->loaded_pages()); 
  ASSERT_EQ(4U, pt->capacity());
  ASSERT_FALSE(pt->page_available(0U));
  ASSERT_FALSE(pt->page_available(1U));
  ASSERT_TRUE(pt->page_available(2U));
  ASSERT_TRUE(pt->page_available(3U));
  ASSERT_TRUE(pt->page_available(4U));
  ASSERT_FALSE(pt->page_available(5U));
  ASSERT_FALSE(pt->page_available(6U));
  ASSERT_TRUE(pt->page_available(7U));
  
  delete pt;

  HW4Environment::AddPoints(30);
}

}  // namespace hw4
