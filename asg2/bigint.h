// $Id: bigint.h,v 1.9 2014-04-11 11:58:33-07 - - $
// Author: Coy Humphrey (cmhumphr)

#ifndef __BIGINT_H__
#define __BIGINT_H__

#include <exception>
#include <iostream>
#include <utility>
using namespace std;

#include "debug.h"

//
// Define class bigint
//
typedef unsigned char digit_t;
typedef vector<digit_t> bigvalue_t;
class bigint {
      friend ostream& operator<< (ostream&, const bigint&);
   private:
      bool negative;
      bigvalue_t big_value;
      typedef pair<bigint,bigint> quotient_remainder;
      quotient_remainder divide (const bigint&) const;
      // Added as private function in order to access big_value
      void div_by_2 (bigint &) const;
      bigvalue_t do_bigadd(const bigvalue_t&, const bigvalue_t&) const;
      bigvalue_t do_bigsub(const bigvalue_t&, const bigvalue_t&) const;
      bigvalue_t do_bigmul(const bigvalue_t&, const bigvalue_t&) const;
      void trim();
   public:
      //
      // Override implicit members.
      //
      bigint();
      bigint (const bigint&);
      bigint& operator= (const bigint&);
      ~bigint();
      //
      // Extra ctors to make bigints.
      //
      bigint (const long);
      bigint (const string&);
      //
      // Basic add/sub operators.
      //
      bigint operator+ (const bigint&) const;
      bigint operator- (const bigint&) const;
      bigint operator-() const;
      long to_long() const;
      //
      // Extended operators implemented with add/sub.
      //
      bigint operator* (const bigint&) const;
      bigint operator/ (const bigint&) const;
      bigint operator% (const bigint&) const;
      //
      // Comparison operators.
      //
      bool operator== (const bigint&) const;
      bool operator<  (const bigint&) const;
};


bigint pow (const bigint& base, const bigint& exponent);

//
// Rest of the comparisons don't need to be members.
//
#define BOOLOPER(OPER,EXPRESSION) \
inline bool operator OPER (const bigint &left, const bigint &right) { \
   return EXPRESSION; \
}
BOOLOPER(!=, not (left == right))
BOOLOPER(> , right < left       )
BOOLOPER(<=, not (right < left) )
BOOLOPER(>=, not (left < right) )

//
// Operators with a left operand long and right operand bigint.
//
#define LONGLEFT(RTYPE,OPER) \
inline RTYPE operator OPER (long left, const bigint& right) { \
   return bigint (left) OPER right; \
}
LONGLEFT(bigint,+)
LONGLEFT(bigint,-)
LONGLEFT(bigint,*)
LONGLEFT(bigint,/)
LONGLEFT(bigint,%)
LONGLEFT(bool,==)
LONGLEFT(bool,<)
LONGLEFT(bool,!=)
LONGLEFT(bool,>)
LONGLEFT(bool,<=)
LONGLEFT(bool,>=)

#endif

