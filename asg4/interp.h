// Author: Coy Humphrey

#ifndef __INTERP_H__
#define __INTERP_H__

#include <iostream>
#include <map>
#include <vector>
using namespace std;

#include "debug.h"
#include "graphics.h"
#include "shape.h"

class interpreter {
   public:
      typedef map <string, shape_ptr> shape_map;
      typedef vector<string> parameters;
      typedef parameters::const_iterator param;
      typedef pair<param,param> range;
      void interpret (const parameters&);
      interpreter() {}
      ~interpreter();

   private:
      interpreter (const interpreter&) = delete;
      interpreter& operator= (const interpreter&) = delete;

      typedef void (interpreter::*interpreterfn)
                   (param begin, param end);
      typedef shape_ptr (interpreter::*factoryfn)
                        (param begin, param end);

      static map<string,interpreterfn> interp_map;
      static map<string,factoryfn> factory_map;
      shape_map objmap;

      void do_define (param begin, param end);
      void do_draw (param begin, param end);
      void do_set_move_speed (param begin, param end);
      void do_border (param begin, param end);

      shape_ptr make_shape (param begin, param end);
      shape_ptr make_text (param begin, param end);
      shape_ptr make_ellipse (param begin, param end);
      shape_ptr make_circle (param begin, param end);
      shape_ptr make_polygon (param begin, param end);
      shape_ptr make_rectangle (param begin, param end);
      shape_ptr make_square (param begin, param end);
      shape_ptr make_line (param begin, param end);
      shape_ptr make_diamond (param begin, param end);
      shape_ptr make_triangle (param begin, param end);
      shape_ptr make_right_triangle (param begin, param end);
      shape_ptr make_isosceles (param begin, param end);
      shape_ptr make_equilateral (param begin, param end);
};

#endif

