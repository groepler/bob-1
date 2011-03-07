/**
 * @file src/cxx/ip/test/crop.cc
 * @author <a href="mailto:Laurent.El-Shafey@idiap.ch">Laurent El Shafey</a> 
 *
 * @brief Test the crop function for 2D and 3D arrays/images
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE IP-Crop Tests
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <blitz/array.h>
#include <stdint.h>
#include <iostream>
#include "core/logging.h"
#include "ip/crop.h"

struct T {
  blitz::Array<uint32_t,2> a2, a2c;
  blitz::Array<uint32_t,3> a3, a3c;

  T(): a2(4,4), a2c(2,2), a3(3,4,4), a3c(3,2,2)  {
    a2 = 0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15;

    a2c = 5, 6, 9, 10;

    a3 = 0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47;

    a3c = 5, 6, 9, 10,
        21, 22, 25, 26,
        37, 38, 41, 42;
  }

  ~T() {}
};

template<typename T, typename U, int d>  
void check_dimensions( blitz::Array<T,d>& t1, blitz::Array<U,d>& t2) 
{
  BOOST_REQUIRE_EQUAL(t1.dimensions(), t2.dimensions());
  for( int i=0; i<t1.dimensions(); ++i)
    BOOST_CHECK_EQUAL(t1.extent(i), t2.extent(i));
}

template<typename T, typename U>  
void checkBlitzEqual( blitz::Array<T,2>& t1, blitz::Array<U,2>& t2)
{
  check_dimensions( t1, t2);
  for( int i=0; i<t1.extent(0); ++i)
    for( int j=0; j<t1.extent(1); ++j)
      BOOST_CHECK_EQUAL(t1(i,j), Torch::core::cast<T>(t2(i,j)));
}

template<typename T, typename U>  
void checkBlitzEqual( blitz::Array<T,3>& t1, blitz::Array<U,3>& t2) 
{
  check_dimensions( t1, t2);
  for( int i=0; i<t1.extent(0); ++i)
    for( int j=0; j<t1.extent(1); ++j)
      for( int k=0; k<t1.extent(2); ++k)
        BOOST_CHECK_EQUAL(t1(i,j,k), Torch::core::cast<T>(t2(i,j,k)));
}

BOOST_FIXTURE_TEST_SUITE( test_setup, T )

BOOST_AUTO_TEST_CASE( test_crop_2d_uint8 )
{
  blitz::Array<uint32_t,2> b2;
  // Full crop
  Torch::ip::crop(a2, b2, 0, 0, 4, 4);
  checkBlitzEqual(a2, b2); 

  // Crop the middle part
  Torch::ip::crop(a2, b2, 1, 1, 2, 2);
  checkBlitzEqual(a2c, b2); 
}
  
BOOST_AUTO_TEST_CASE( test_crop_3d_uint8 )
{
  blitz::Array<uint32_t,3> b3;
  // Full crop
  Torch::ip::crop(a3, b3, 0, 0, 4, 4);
  checkBlitzEqual(a3, b3); 

  // Crop the middle part
  Torch::ip::crop(a3, b3, 1, 1, 2, 2);
  checkBlitzEqual(a3c, b3); 
}

BOOST_AUTO_TEST_SUITE_END()

