// Author: Coy Humphrey

#include <typeinfo>
#include <unordered_map>
#include <cmath>
using namespace std;

#include "shape.h"
#include "util.h"

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font, const string& textdata):
      glut_bitmap_font(glut_bitmap_font), textdata(textdata) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}


polygon::polygon (const vertex_list& vertices_): vertices(vertices_) {
   DEBUGF ('c', this);
   GLfloat x_avg = 0.0;
   GLfloat y_avg = 0.0;
   for (auto itor = vertices.begin(); itor != vertices.end(); ++itor)
   {
      x_avg += itor->xpos;
      y_avg += itor->ypos;
   }
   x_avg /= vertices.size();
   y_avg /= vertices.size();
   for (auto itor = vertices.begin(); itor != vertices.end(); ++itor)
   {
      itor->xpos -= x_avg;
      itor->ypos -= y_avg;
   }
}

rectangle::rectangle (GLfloat width, GLfloat height):
   polygon({vertex{0, 0}, vertex{width, 0}, vertex{width, height},
      vertex{0, height}}) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

diamond::diamond (GLfloat width, GLfloat height):
   polygon({vertex{-width,0}, vertex{0,-height}, vertex{width,0},
                   vertex{0, height}})
{}

triangle::triangle (const vertex_list& vertices_):
   polygon (vertices_) {}
   
right_triangle::right_triangle (GLfloat width, GLfloat height):
   triangle ({vertex{0,0}, vertex{width,0}, vertex{0, height}}) {}

isosceles::isosceles (GLfloat width, GLfloat height):
   triangle ({vertex{-width/2, 0}, vertex{0, height}, 
      vertex{width/2, 0}}) {}
      
equilateral::equilateral (GLfloat width):
   isosceles (width, width) {}
   
void text::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   glColor3ub (color.red, color.green, color.blue);
   glRasterPos2i (center.xpos, center.ypos);
   glutBitmapString (glut_bitmap_font, (GLubyte*) textdata.c_str());
   glLoadIdentity();
}
void text::draw_outline (const vertex& center, const rgbcolor& color)
   const 
{
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   glColor3ub (color.red, color.green, color.blue);
   glRasterPos2i (center.xpos, center.ypos);
   glutBitmapString (glut_bitmap_font, (GLubyte*) textdata.c_str());
   glLoadIdentity();
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   // move to origin
   glLoadIdentity();
   glTranslatef (center.xpos, center.ypos, 0.0f);
   glBegin (GL_POLYGON);
   glEnable (GL_LINE_SMOOTH);
   glColor3ub (color.red, color.green, color.blue);
   for (int i = 0; i < 360; ++i)
   {
      GLfloat xpos = dimension.xpos/2 * cos (i * M_PI / 180);
      GLfloat ypos = dimension.ypos/2 * sin (i * M_PI / 180);
      glVertex2f (xpos, ypos);
   }
   glEnd();
   glLoadIdentity();
}

void ellipse::draw_outline (const vertex& center, const rgbcolor& color)
   const 
{
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   // move to origin
   glLoadIdentity();
   glTranslatef (center.xpos, center.ypos, 0.0f);
   glBegin (GL_LINE_LOOP);
   glEnable (GL_LINE_SMOOTH);
   glColor3ub (color.red, color.green, color.blue);
   for (int i = 0; i < 360; ++i)
   {
      GLfloat xpos = dimension.xpos/2 * cos (i * M_PI / 180);
      GLfloat ypos = dimension.ypos/2 * sin (i * M_PI / 180);
      glVertex2f (xpos, ypos);
   }
   glEnd();
   glLoadIdentity();
}

void polygon::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   // move to origin
   glLoadIdentity();
   glTranslatef (center.xpos, center.ypos, 0.0f);
   glBegin (GL_POLYGON);
   glColor3ub (color.red, color.green, color.blue);
   for (auto itor : vertices)
   {
      glVertex2f (itor.xpos, itor.ypos);
   }
   glEnd();
   glLoadIdentity();
}

void polygon::draw_outline (const vertex& center, const rgbcolor& color)
   const 
{
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   // move to origin
   glLoadIdentity();
   glTranslatef (center.xpos, center.ypos, 0.0f);
   glBegin (GL_LINE_LOOP);
   glColor3ub (color.red, color.green, color.blue);
   for (auto itor : vertices)
   {
      glVertex2f (itor.xpos, itor.ypos);
   }
   glEnd();
   glLoadIdentity();
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

