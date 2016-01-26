// $Id: rgbcolor.h,v 1.6 2014-05-21 15:44:26-07 - - $

#ifndef __RGBCOLOR_H__
#define __RGBCOLOR_H__

#include <string>
#include <unordered_map>
using namespace std;

#include <GL/freeglut.h>

typedef unsigned char uchar;
struct rgbcolor {
   union {
      uchar ubvec[3];
      struct {
         uchar red;
         uchar green;
         uchar blue;
      };
   };
   explicit rgbcolor(): red(0), green(0), blue(0) {}
   explicit rgbcolor (uchar red, uchar green, uchar blue):
               red(red), green(green), blue(blue) {}
   explicit rgbcolor (const string&);
   const GLubyte* ubvec3() { return ubvec; }
   operator string() const;
};

ostream& operator<< (ostream&, const rgbcolor&);

extern const std::unordered_map<std::string,rgbcolor> color_names;

#endif

