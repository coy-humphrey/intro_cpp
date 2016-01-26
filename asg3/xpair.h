// $Id: xpair.h,v 1.3 2014-04-24 18:02:55-07 - - $

#ifndef __XPAIR_H__
#define __XPAIR_H__

#include <iostream>

using namespace std;

//
// Class xpair works like pair(c++).
//
// The implicitly generated members will work, because they just
// send messages to the first and second fields, respectively.
// Caution:  xpair() does not initialize its fields unless
// First and Second do so with their default ctors.
//

template <typename First, typename Second>
struct xpair {
   First first;
   Second second;
   xpair(): first(First()), second(Second()) {}
   xpair (const First& first, const Second& second):
               first(first), second(second) {}
};

template <typename First, typename Second>
ostream& operator<< (ostream& out, const xpair<First,Second>& pair) {
   out << "[" << pair.first << "," << pair.second << "]";
   return out;
}

#endif

