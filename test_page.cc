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

#include <iostream>
#include <fstream>

#include "gtest/gtest.h"
#include "./test_suite.h"
#include "./Page.h"

using std::fstream;
using std::ios_base;
using simplevm::Page;

namespace hw4 {

class Test_Page : public ::testing::Test {
 protected:
  // Code here will be called before all tests execute (ie, before
  // any TEST_F).
  virtual void SetUp() {
    // Do nothing
  }

  // Code here will be called after each test executes (ie, after
  // each TEST_F)
  virtual void TearDown() {
    // Do nothing
  }

 public:
  // These values are used as constants for the Page tests.
  // They cannot be const, as stored value pointers are non-const.
  static constexpr const char* kSimpleFileName = "./test_files/simple.bytes";
  static constexpr const char* kComplexFileName = "./test_files/complex.bytes";

};  // class Test_Page

// statics

TEST_F(Test_Page, basic_ctor) {
  HW4Environment::OpenTestCase();
  fstream s(kSimpleFileName, ios_base::in);
  Page* page = new Page(s, 0);
  ASSERT_FALSE(page->dirty());
  ASSERT_EQ(0, page->pno());
  delete page;

  HW4Environment::AddPoints(5);

  page = new Page(s, 3);
  ASSERT_FALSE(page->dirty());
  ASSERT_EQ(3, page->pno());
  delete page;

  HW4Environment::AddPoints(5);
}

TEST_F(Test_Page, access) {
  HW4Environment::OpenTestCase();
  fstream s(kSimpleFileName, ios_base::in);
  Page* page = new Page(s, 0);
  ASSERT_FALSE(page->dirty());
  ASSERT_EQ(0, page->pno());
  uint32_t i;

  for (i = 0; i < Page::PAGE_SIZE / 4; i++) {
    ASSERT_EQ(i, page->access<uint32_t>(i * sizeof(uint32_t)));
  }
  ASSERT_FALSE(page->dirty());
  ASSERT_EQ(0, page->pno());
  delete page;

  HW4Environment::AddPoints(5);

  page = new Page(s, 2);
  for (i = 2*Page::PAGE_SIZE; i < 3*Page::PAGE_SIZE; i+=4) {
    uint8_t expected;
    expected = static_cast<uint8_t>((i/4) & 0xFF);
    ASSERT_EQ(expected, page->access<uint8_t>(i));

    expected = static_cast<uint8_t>(((i/4) >> 8) & 0xFF);
    ASSERT_EQ(expected, page->access<uint8_t>(i+1));

    expected = static_cast<uint8_t>(((i/4) >> 16) & 0xFF);
    ASSERT_EQ(expected, page->access<uint8_t>(i+2));

    expected = static_cast<uint8_t>(((i/4) >> 24) & 0xFF);
    ASSERT_EQ(expected, page->access<uint8_t>(i+3));
  }
  ASSERT_FALSE(page->dirty());
  ASSERT_EQ(2, page->pno());
  delete page;

  HW4Environment::AddPoints(10);

  fstream c(kComplexFileName, ios_base::in);
  page = new Page(c, 0);
  ASSERT_FALSE(page->access<bool>(0));

  ASSERT_EQ(43264, page->access<uint16_t>(2022));
  ASSERT_FALSE(page->dirty());
  ASSERT_EQ(0, page->pno());
  delete page;

  HW4Environment::AddPoints(10);

  page = new Page(c, 23);
  for (uint64_t index = 23 * Page::PAGE_SIZE; index < 24 * Page::PAGE_SIZE; index+=2) {
    uint64_t square = (index/2) * (index/2);
    uint16_t value = ((square & 0x00FF) << 8) | ((square >> 48) & 0x00FF);
    ASSERT_EQ(value, page->access<uint16_t>(index));
  }
  ASSERT_FALSE(page->dirty());
  ASSERT_EQ(23, page->pno());
  delete page;

  HW4Environment::AddPoints(10);
  
}

TEST_F(Test_Page, store) {
  HW4Environment::OpenTestCase();
  fstream s(kSimpleFileName, ios_base::in);

  // create a copy of the file so that
  // the original test files are not modified
  fstream copy;
  copy.open("./test_files/temp.bytes", ios_base::in | ios_base::out | ios_base::trunc);
  char bytes[Page::PAGE_SIZE];
  s.read(bytes, Page::PAGE_SIZE);
  while (!s.eof()) {
    copy.write(bytes, Page::PAGE_SIZE);
    s.read(bytes, Page::PAGE_SIZE);
  }

  Page* page = new Page(copy, 3);
  for (uint32_t i = 0; i < Page::PAGE_SIZE / 4; i++) {
    page->store<uint32_t>(3*Page::PAGE_SIZE + i * sizeof(uint32_t), i+1);
    ASSERT_EQ(i+1, page->access<uint32_t>(3 * Page::PAGE_SIZE + i * sizeof(uint32_t)));
    ASSERT_TRUE(page->dirty());
  }
  ASSERT_EQ(3, page->pno());
  HW4Environment::AddPoints(10);

  // check that the contents haven't been flushed to the page yet
  uint32_t values[1024];
  copy.seekg(Page::PAGE_SIZE * 3, ios_base::beg);
  copy.read(reinterpret_cast<char*>(values), Page::PAGE_SIZE);
  for (uint32_t i = 0; i < Page::PAGE_SIZE / 4; i++) {
    ASSERT_EQ(i+3*Page::PAGE_SIZE/4, values[i]);
  }

  // flush the contents and then check that they are updated
  page->flush();
  ASSERT_FALSE(page->dirty());
  copy.seekg(Page::PAGE_SIZE * 3, ios_base::beg);
  copy.read(reinterpret_cast<char*>(values), Page::PAGE_SIZE);
  for (uint32_t i = 0; i < Page::PAGE_SIZE / 4; i++) {
    ASSERT_EQ(i+1, values[i]);
  }
  delete page;
  HW4Environment::AddPoints(15);

  // do a similar test for the complex file
  copy.close();
  copy.open("./test_files/temp.bytes", ios_base::in | ios_base::out | ios_base::trunc);
  fstream c(kComplexFileName, ios_base::in);
  c.read(bytes, Page::PAGE_SIZE);
  while (!c.eof()) {
    copy.write(bytes, Page::PAGE_SIZE);
    c.read(bytes, Page::PAGE_SIZE);
  }

  // cache the original values stored in page 15
  uint8_t original[Page::PAGE_SIZE];
  copy.seekg(Page::PAGE_SIZE * 15, ios_base::beg);
  copy.read(reinterpret_cast<char*>(original), Page::PAGE_SIZE);

  page = new Page(copy, 15);
  for (int32_t i = Page::PAGE_SIZE-1; i >= 0; i--) {
    page->store<uint8_t>(15*Page::PAGE_SIZE + static_cast<uint32_t>(i), i % 256);
    ASSERT_EQ(i%256, page->access<uint8_t>(15*Page::PAGE_SIZE + i));
    ASSERT_TRUE(page->dirty());
  }
  ASSERT_EQ(15, page->pno());
  HW4Environment::AddPoints(10);

  // check that the contents haven't been flushed to the page yet
  uint8_t byte_values[Page::PAGE_SIZE];
  copy.seekg(Page::PAGE_SIZE * 15, ios_base::beg);
  copy.read(reinterpret_cast<char*>(byte_values), Page::PAGE_SIZE);
  for (uint32_t i = 0; i < Page::PAGE_SIZE; i++) {
    ASSERT_EQ(original[i], byte_values[i]);
  }

  // flush the contents of the page to disk by deleting
  // instead of calling flush
  delete page;

  copy.seekg(Page::PAGE_SIZE * 15, ios_base::beg);
  copy.read(reinterpret_cast<char*>(byte_values), Page::PAGE_SIZE);
  for (uint32_t i = 0; i < Page::PAGE_SIZE; i++) {
    ASSERT_EQ(i % 256, byte_values[i]);
  }
  HW4Environment::AddPoints(15);
}

TEST_F(Test_Page, copying) {
  HW4Environment::OpenTestCase();
  fstream simple(kSimpleFileName, ios_base::in);

  // create a copy of the file so that
  // the original test files are not modified
  fstream copy;
  copy.open("./test_files/temp.bytes", ios_base::in | ios_base::out | ios_base::trunc);

  char bytes[Page::PAGE_SIZE];
  simple.read(bytes, Page::PAGE_SIZE);
  while (!simple.eof()) {
    copy.write(bytes, Page::PAGE_SIZE);
    simple.read(bytes, Page::PAGE_SIZE);
  }

  Page* page1 = new Page(copy, 1);
  Page* page2 = new Page(*page1);
  ASSERT_FALSE(page1->dirty());
  ASSERT_EQ(1, page1->pno());
  ASSERT_FALSE(page2->dirty());
  ASSERT_EQ(1, page2->pno());

  HW4Environment::AddPoints(5);

  ASSERT_EQ(1176U, page1->access<uint32_t>(4704));
  ASSERT_EQ(1176U, page2->access<uint32_t>(4704));

  page1->store<uint32_t>(4704, 152U);
  ASSERT_EQ(152U, page1->access<uint32_t>(4704));
  ASSERT_EQ(1176U, page2->access<uint32_t>(4704));
  ASSERT_TRUE(page1->dirty());
  ASSERT_EQ(1, page1->pno());
  ASSERT_FALSE(page2->dirty());
  ASSERT_EQ(1, page2->pno());

  delete page1;
  delete page2;

  HW4Environment::AddPoints(5);
}


}  // namespace hw4
