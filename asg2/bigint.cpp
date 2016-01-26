// $Id: bigint.cpp,v 1.55 2014-04-09 17:03:58-07 - - $
// Author: Coy Humphrey (cmhumphr)

#include <cassert>
#include <cstdlib>
#include <exception>
#include <limits>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"

#define CDTOR_TRACE DEBUGF ('~', this << " -> " << *this)


bigint::bigint()
: negative {false}, big_value {} 
{
   CDTOR_TRACE;
}

bigint::bigint (const bigint& that)
: negative (that.negative), big_value (that.big_value) 
{
   *this = that;
   CDTOR_TRACE;
}

bigint& bigint::operator= (const bigint& that) 
{
   if (this == &that) return *this;
   this->negative = that.negative;
   this->big_value = that.big_value;
   return *this;
}

// No memory assigned with new
bigint::~bigint() 
{
   CDTOR_TRACE;
}

bigint::bigint (long that)
: big_value {}
{
   negative = that < 0 ? true : false;
   while (that != 0)
   {
      big_value.push_back (that % 10);
      that = that / 10;
   }
   trim();
   CDTOR_TRACE;
}

bigint::bigint (const string& that) 
: negative {false}, big_value {}
{
   assert (that.size() > 0);
   // Use reverse itor
   // Start with least significant digit in string and pop all digits
   // into big_value. big_value will then be in order of least to most
   // signicant digits.
   auto itor = that.crbegin();
   while (itor != that.rend()) 
   {
      if (*itor == '_')
      {
         negative = true;
         ++itor;
         continue;
      }
      big_value.push_back (*itor++ - '0');
   }
   trim();
   if (big_value.size() == 0) negative = false;
   CDTOR_TRACE;
}



bool do_bigless (const bigvalue_t& left, const bigvalue_t& right) 
{
   if (left.size() < right.size()) return true;
   if (left.size() > right.size()) return false;
   for (int itor = left.size() - 1; itor > -1; --itor)
   {
      if (left[itor] < right[itor]) return true;
      if (left[itor] > right[itor]) return false;
   }
   return false;
}

bigint bigint::operator+ (const bigint& that) const 
{
   bigint result;
   if (negative == that.negative)
   {
      bigvalue_t tmp = do_bigadd (big_value, that.big_value);
      result.big_value.assign (tmp.begin(), tmp.end());
      result.negative = negative;
   }
   else
   {
      if (do_bigless (big_value, that.big_value))
      {
         bigvalue_t tmp = do_bigsub (that.big_value, big_value);
         result.big_value.assign (tmp.begin(), tmp.end());
         result.negative = that.negative;
      }
      else
      {
         bigvalue_t tmp = do_bigsub (big_value, that.big_value);
         result.big_value.assign (tmp.begin(), tmp.end());
         result.negative = negative;
      }
   }
   result.trim();
   if (result.big_value.size() == 0) result.negative = false;
   return result;
}

bigint bigint::operator- (const bigint& that) const 
{
   bigint result {};
   if (negative != that.negative)
   {
      bigvalue_t tmp = do_bigadd (big_value, that.big_value);
      result.big_value.assign (tmp.begin(), tmp.end());
      result.negative = negative;
   }
   else
   {
      if (do_bigless (big_value, that.big_value))
      {
         bigvalue_t tmp = do_bigsub (that.big_value, big_value);
         result.big_value.assign (tmp.begin(), tmp.end());
         result.negative = !negative;
      }
      else
      {
         bigvalue_t tmp = do_bigsub (big_value, that.big_value);
         result.big_value.assign (tmp.begin(), tmp.end());
         result.negative = negative;
      }
   }
   result.trim();
   if (result.big_value.size() == 0) result.negative = false;
   return result;
}

bigint bigint::operator-() const 
{
   bigint value(*this);
   value.negative = !negative;
   if (value.big_value.size() == 0) value.negative = false;
   return value;
}

long bigint::to_long() const 
{
   if (*this <= bigint (numeric_limits<long>::min())
    or *this > bigint (numeric_limits<long>::max()))
               throw range_error ("to_long: out of range");
   
   long value = 0;
   auto itor = big_value.crbegin();
   while (itor != big_value.rend())
   {
      value *= 10;
      value += *itor;
      ++itor;
   }
   
   return negative ? - value : + value;
}


//
// Multiplication algorithm.
//
bigint bigint::operator* (const bigint& that) const 
{
   bigint result {};
   bigvalue_t tmp = do_bigmul (big_value, that.big_value);
   result.big_value.assign (tmp.begin(), tmp.end());
   result.trim();
   result.negative = (negative == that.negative) ? false : true;
   if (result.big_value.size() == 0) result.negative = false;
   return result;
}

//
// Division algorithm.
//

void mul_by_2 (bigint &big_value) 
{
   big_value = big_value * 2;
}

// Uses decimal divide by 2 algorithm found at:
// en.wikipedia.org/wiki/Division_by_two#Decimal
void bigint::div_by_2 (bigint &big_value) const
{
   big_value.big_value.push_back (0);
   auto itor = big_value.big_value.crbegin();
   auto itor2 = big_value.big_value.crbegin();
   ++itor2;
   bigvalue_t result {};
   while (itor2 != big_value.big_value.rend())
   {
      if (*itor % 2 == 0)
         if (*itor2 < 2)
            result.push_back (0);
         else if (*itor2 < 4)
            result.push_back (1);
         else if (*itor2 < 6)
            result.push_back (2);
         else if (*itor2 < 8)
            result.push_back (3);
         else
            result.push_back (4);
      else
         if (*itor2 < 2)
            result.push_back (5);
         else if (*itor2 < 4)
            result.push_back (6);
         else if (*itor2 < 6)
            result.push_back (7);
         else if (*itor2 < 8)
            result.push_back (8);
         else
            result.push_back (9);
      ++itor;
      ++itor2;
   }
   big_value.big_value.assign (result.rbegin(), result.rend());
   big_value.trim();
}


bigint::quotient_remainder bigint::divide (const bigint& that) const 
{
   if (that == 0) throw domain_error ("divide by 0");
   typedef bigint unumber;
   static unumber zero (0);
   if (that == 0) throw domain_error ("bigint::divide");
   unumber divisor = that;
   divisor.negative = false;
   unumber quotient (0);
   unumber remainder = *this;
   remainder.negative = false;
   unumber power_of_2 (1);
   while (do_bigless (divisor.big_value, remainder.big_value)) {
      mul_by_2 (divisor);
      mul_by_2 (power_of_2);
   }
   while (do_bigless (zero.big_value, power_of_2.big_value)) {
      if (not do_bigless (remainder.big_value, divisor.big_value)) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      div_by_2 (divisor);
      div_by_2 (power_of_2);
   }
   quotient.negative  = (negative == divisor.negative) ? false : true;
   remainder.negative = (negative == divisor.negative) ? false : true;
   if ( quotient.big_value.size() == 0)  quotient.negative = false;
   if (remainder.big_value.size() == 0) remainder.negative = false;
   return {quotient, remainder};
}

bigint bigint::operator/ (const bigint& that) const 
{
   return divide (that).first;
}

bigint bigint::operator% (const bigint& that) const 
{
   return divide (that).second;
}

bool bigint::operator== (const bigint& that) const 
{
   if (that.negative != negative) return false;
   if (that.big_value.size() != big_value.size()) return false;
   for (size_t itor = 0; itor < big_value.size(); ++itor)
   {
      if (that.big_value[itor] != big_value[itor]) return false;
   }
   return true;
}

bool bigint::operator< (const bigint& that) const 
{
   
   // If this is negative and that is positive, this < that
   if (negative && !that.negative) return true;
   // If this is positive and that is negative, this !< that
   if (!negative && that.negative) return false;
   if (negative)
   {
      // If this is negative and is longer than that, this < that
      if (big_value.size() > that.big_value.size()) return true;
      // If this is negative and is shorter than that, this !< that
      if (big_value.size() < that.big_value.size()) return false;
      for (int itor = big_value.size() - 1; itor > -1; --itor)
      {
         // If this has a greater abs value, this < that
         if (big_value[itor] > that.big_value[itor]) return true;
         // If that has a greater abs value, this !< that
         if (big_value[itor] < that.big_value[itor]) return false;
      }
   }
   else
   {
      // If this is positive and 
      if (big_value.size() < that.big_value.size()) return true;
      if (big_value.size() > that.big_value.size()) return false;
      for (int itor = big_value.size() - 1; itor > -1; --itor)
      {
         // If this has a smaller abs value, this < that
         if (big_value[itor] < that.big_value[itor]) return true;
         // If that has a smaller abs value, this !< that
         if (big_value[itor] > that.big_value[itor]) return false;
      }
   }
   
   return true;
}

bigvalue_t bigint::do_bigadd 
   (const bigvalue_t &left, const bigvalue_t &right) const
{
   bigvalue_t result{};
   auto litor = left.cbegin();
   auto ritor = right.cbegin();
   digit_t carry = 0;
   while (litor != left.end() || ritor != right.end())
   {
      digit_t ldig = 0;
      digit_t rdig = 0;
      if (litor != left.end())
      {
         ldig = *litor;
         ++litor;
      }
      if (ritor != right.end())
      {
         rdig = *ritor;
         ++ritor;
      }
      digit_t dig = ldig + rdig + carry;
      if (dig >= 10)
      {
         carry = 1;
         dig -= 10;
      }
      else
      {
         carry = 0;
      }
      result.push_back (dig);
   }
   if (carry == 1) result.push_back (carry);
   return result;
}

bigvalue_t bigint::do_bigsub 
   (const bigvalue_t &left, const bigvalue_t &right) const
{
   bigvalue_t result{};
   auto litor = left.cbegin();
   auto ritor = right.cbegin();
   char carry = 0;
   while (litor != left.end() || ritor != right.end())
   {
      digit_t ldig = 0;
      digit_t rdig = 0;
      if (litor != left.end())
      {
         ldig = *litor;
         ++litor;
      }
      if (ritor != right.end())
      {
         rdig = *ritor;
         ++ritor;
      }
      char dig = ldig - rdig + carry;
      if (dig < 0)
      {
         carry = -1;
         dig += 10;
      }
      else
      {
         carry = 0;
      }
      result.push_back (dig);
   }
   return result;
}
// Follows the algorithm described in section 5j of the assignment
bigvalue_t bigint::do_bigmul 
   (const bigvalue_t &left, const bigvalue_t &right) const
{
   bigvalue_t result (left.size() + right.size(), 0);
   for (size_t litor = 0;litor < left.size(); ++litor)
   {
      digit_t c = 0;
      for (size_t ritor = 0;ritor < right.size(); ++ritor)
      {
         digit_t d = result[ritor+litor] + left[litor]*right[ritor] + c;
         result[ritor+litor] = d % 10;
         c = d / 10;
      }
      result[litor + right.size()] = c;
   }
   return result;
}

void bigint::trim()
{
   while (big_value.size() > 0 && big_value.back() == 0)
   {
      big_value.pop_back();
   }
}

ostream& operator<< (ostream& out, const bigint& that) 
{
   // Number of characters dc displays per line
   int perline = 69;
   int count = 0;
   if (that.negative)
   {
      out << "-";
      ++count;
   }
   auto itor = that.big_value.crbegin();
   while (itor != that.big_value.rend())
   {
      out << (static_cast<char>(*itor++ + '0'));
      ++count;
      if (count == perline)
      {
         cout << "\\" << endl;
         count = 0;
      } 
   }
   if (that.big_value.size() == 0)
   {
      out << '0';
   }
   return out;
}


bigint pow (const bigint& base, const bigint& exponent) 
{
   DEBUGF ('^', "base = " << base << ", exponent = " << exponent);
   if (base == 0) return 0;
   bigint base_copy = base;
   long expt = exponent.to_long();
   bigint result = 1;
   if (expt < 0) {
      base_copy = 1 / base_copy;
      expt = - expt;
   }
   while (expt > 0) {
      if (expt & 1) { //odd
         result = result * base_copy;
         --expt;
      }else { //even
         base_copy = base_copy * base_copy;
         expt /= 2;
      }
   }
   DEBUGF ('^', "result = " << result);
   return result;
}
