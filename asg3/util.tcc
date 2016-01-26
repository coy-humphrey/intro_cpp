// $Id: util.tcc,v 1.2 2014-04-24 18:14:51-07 - - $

#include <sstream>
#include <typeinfo>
using namespace std;

template <typename item_t>
ostream& operator<< (ostream& out, const list<item_t>& vec) {
   bool want_space = false;
   for (const auto& item: vec) {
      if (want_space) cout << " ";
      cout << item;
      want_space = true;
   }
   return out;
}

template <typename type_t>
string to_string (const type_t& that) {
   ostringstream stream;
   stream << that;
   return stream.str();
}

template <typename type_t>
type_t from_string (const string& that) {
   stringstream stream;
   stream << that;
   type_t result;
   if (not (stream >> result   // Can we read type from string?
        and stream >> std::ws  // Flush trailing white space.
        and stream.eof())      // Must now be at end of stream.
   ) {
      throw domain_error (string (typeid (type_t).name())
            + " from_string (" + that + ")");
   }
   return result;
}

