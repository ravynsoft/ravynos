/**************************************************************************
 *
 * (C) Copyright VMware, Inc 2010.
 * (C) Copyright John Maddock 2006.
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 **************************************************************************/


/*
 * This file allows to compute the minimax polynomial coefficients we use
 * for fast exp2/log2.
 *
 * How to use this source:
 *
 * - Download and build the NTL library from
 *   http://shoup.net/ntl/download.html , or install libntl-dev package if on
 *   Debian.
 *
 * - Download boost source code matching to your distro. 
 *
 * - Goto libs/math/minimax and replace f.cpp with this file.
 *
 * - Build as
 *
 *   g++ -o minimax -I /path/to/ntl/include main.cpp f.cpp /path/to/ntl/src/ntl.a
 *
 * - Run as 
 *
 *    ./minimax
 *
 * - For example, to compute exp2 5th order polynomial between [0, 1] do:
 *
 *    variant 0
 *    range 0 1
 *    order 5 0
 *    step 200
 *    info
 *
 *  and take the coefficients from the P = { ... } array.
 *
 * - To compute log2 4th order polynomial between [0, 1/9] do:
 *
 *    variant 1
 *    range 0 0.111111112
 *    order 4 0
 *    step 200
 *    info
 *
 * - For more info see
 * http://www.boost.org/doc/libs/1_47_0/libs/math/doc/sf_and_dist/html/math_toolkit/toolkit/internals2/minimax.html
 */

#define L22
#include <boost/math/bindings/rr.hpp>
#include <boost/math/tools/polynomial.hpp>

#include <cmath>

boost::math::ntl::RR exp2(const boost::math::ntl::RR& x)
{
      return exp(x*log(2.0));
}

boost::math::ntl::RR log2(const boost::math::ntl::RR& x)
{
      return log(x)/log(2.0);
}

boost::math::ntl::RR f(const boost::math::ntl::RR& x, int variant)
{
   switch(variant)
   {
   case 0:
      return exp2(x);

   case 1:
      return log2((1.0 + sqrt(x))/(1.0 - sqrt(x)))/sqrt(x);
   }

   return 0;
}


void show_extra(
   const boost::math::tools::polynomial<boost::math::ntl::RR>& n, 
   const boost::math::tools::polynomial<boost::math::ntl::RR>& d, 
   const boost::math::ntl::RR& x_offset, 
   const boost::math::ntl::RR& y_offset, 
   int variant)
{
   switch(variant)
   {
   default:
      // do nothing here...
      ;
   }
}

