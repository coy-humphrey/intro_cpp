// Author: Coy Humphrey

#include <map>
#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"
#include "util.h"

map<string,interpreter::interpreterfn> interpreter::interp_map {
   {"define" , &interpreter::do_define        },
   {"draw"   , &interpreter::do_draw          },
   {"moveby" , &interpreter::do_set_move_speed},
   {"border" , &interpreter::do_border        },
};

map<string,interpreter::factoryfn> interpreter::factory_map {
   {"text"           , &interpreter::make_text          },
   {"ellipse"        , &interpreter::make_ellipse       },
   {"circle"         , &interpreter::make_circle        },
   {"polygon"        , &interpreter::make_polygon       },
   {"rectangle"      , &interpreter::make_rectangle     },
   {"square"         , &interpreter::make_square        },
   {"diamond"        , &interpreter::make_diamond       },
   {"triangle"       , &interpreter::make_triangle      },
   {"right_triangle" , &interpreter::make_right_triangle},
   {"isosceles"      , &interpreter::make_isosceles     },
   {"equilateral"    , &interpreter::make_equilateral   },
};

interpreter::~interpreter() {
   for (const auto& itor: objmap) {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

void interpreter::interpret (const parameters& params) {
   DEBUGF ('i', params);
   param begin = params.cbegin();
   string command = *begin;
   auto itor = interp_map.find (command);
   if (itor == interp_map.end()) throw runtime_error ("syntax error");
   interpreterfn func = itor->second;
   (this->*func) (++begin, params.cend());
}

void interpreter::do_define (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string name = *begin;
   objmap.insert ({name, make_shape (++begin, end)});
}


void interpreter::do_draw (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 4) throw runtime_error ("syntax error");
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find (name);
   if (itor == objmap.end()) {
      throw runtime_error (name + ": no such shape");
   }
   vertex where {from_string<GLfloat> (begin[2]),
                 from_string<GLfloat> (begin[3])};
   rgbcolor color {begin[0]};
   window::push_back (object (itor->second, where, color));
   //itor->second->draw (where, color);
}

void interpreter::do_set_move_speed (param begin, param end)
{
   if (end - begin != 1) throw runtime_error ("syntax error");
   GLfloat movespeed = from_string<GLfloat> (begin[0]);
   window::move_speed = movespeed;
}

void interpreter::do_border (param begin, param end)
{
   if (end - begin != 2)
   {
      throw runtime_error ("syntax error");
   }
   rgbcolor col (begin[0]);
   GLfloat width = from_string<GLfloat> (begin[1]);
   window::border_col = col;
   window::border_width = width;
}

shape_ptr interpreter::make_shape (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string type = *begin++;
   auto itor = factory_map.find(type);
   if (itor == factory_map.end()) {
      throw runtime_error (type + ": no such shape");
   }
   factoryfn func = itor->second;
   return (this->*func) (begin, end);
}

shape_ptr interpreter::make_text (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin < 1)
   {
      throw runtime_error ("make_text: too few arguments");
   }
   auto itor = fontcode.find (begin[0]);
   if (itor == fontcode.end())
   {
      throw runtime_error (begin[0] + ": no such font");
   }
   void *font = itor->second;
   string text_ = "";
   bool beginning = true;
   for (auto itor = begin + 1; itor != end; ++itor)
   {
      if (beginning) beginning = false;
      else text_ += " ";
      text_ += *itor;
   }
   return make_shared<text> (font, text_);
}

shape_ptr interpreter::make_ellipse (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin < 2)
   {
      throw runtime_error ("make_ellipse: too few arguments");
   }
   GLfloat width = from_string<GLfloat> (begin[0]);
   GLfloat height = from_string<GLfloat> (begin[1]);
   return make_shared<ellipse> (width, height);
}

shape_ptr interpreter::make_circle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin < 1)
   {
      throw runtime_error ("make_circle: too few arguments");
   }
   GLfloat diameter = from_string<GLfloat> (begin[0]);
   return make_shared<circle> (diameter);
}

shape_ptr interpreter::make_polygon (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   int len = end-begin;
   if (len < 2)
   {
      throw runtime_error ("make_polygon: too few arguments");
   }
   else if (len % 2 != 0)
   {
      throw runtime_error ("make_polygon: incomplete (x,y) coordinate");
   }
   vertex_list result;
   // len is even
   // if i is even, i < len -> (i + 1) < len
   for (int i = 0; i < len; i += 2)
   {
      GLfloat x = from_string<GLfloat> (begin[i]);
      GLfloat y = from_string<GLfloat> (begin[i+1]);
      result.push_back (vertex {x, y});
   }
   return make_shared<polygon> (result);
}

shape_ptr interpreter::make_rectangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin < 2)
   {
      throw runtime_error ("make_rectangle: too few arguments");
   }
   GLfloat width = from_string<GLfloat> (begin[0]);
   GLfloat height = from_string<GLfloat> (begin[1]);
   return make_shared<rectangle> (width, height);
}

shape_ptr interpreter::make_square (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin < 1)
   {
      throw runtime_error ("make_square: too few arguments");
   }
   GLfloat width = from_string<GLfloat> (begin[0]);
   return make_shared<square> (width);
}

shape_ptr interpreter::make_diamond (param begin, param end)
{
   if (end - begin < 2)
   {
      throw runtime_error ("make_diamond: too few arguments");
   }
   GLfloat width = from_string<GLfloat> (begin[0]);
   GLfloat height = from_string<GLfloat> (begin[1]);
   return make_shared<diamond> (width, height);
}

shape_ptr interpreter::make_triangle (param begin, param end)
{
   if (end - begin != 6)
   {
      throw runtime_error ("make_triangle: too few arguments");
   }
   
   // Same as make_polygon
   vertex_list result;
   for (int i = 0; i < 6; i += 2)
   {
      GLfloat x = from_string<GLfloat> (begin[i]);
      GLfloat y = from_string<GLfloat> (begin[i+1]);
      result.push_back (vertex {x, y});
   }
   return make_shared<triangle> (result);
}

shape_ptr interpreter::make_right_triangle (param begin, param end)
{
   if (end - begin != 2)
   {
      throw runtime_error ("make_right_triangle: too few arguments");
   }
   GLfloat width = from_string<GLfloat> (begin[0]);
   GLfloat height = from_string<GLfloat> (begin[1]);
   return make_shared<right_triangle> (width, height);
}

shape_ptr interpreter::make_isosceles (param begin, param end)
{
   if (end - begin != 2)
   {
      throw runtime_error ("make_isosceles: too few arguments");
   }
   GLfloat width = from_string<GLfloat> (begin[0]);
   GLfloat height = from_string<GLfloat> (begin[1]);
   return make_shared<isosceles> (width, height);
}

shape_ptr interpreter::make_equilateral (param begin, param end)
{
   if (end - begin != 1)
   {
      throw runtime_error ("make_equilateral: too few arguments");
   }
   GLfloat width = from_string<GLfloat> (begin[0]);
   return make_shared<equilateral> (width);
}
