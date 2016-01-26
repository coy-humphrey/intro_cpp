// Author: Coy Humphrey

#include <iostream>
using namespace std;

#include <GL/freeglut.h>

#include "graphics.h"
#include "util.h"

int window::width = 640; // in pixels
int window::height = 480; // in pixels
vector<object> window::objects;
size_t window::selected_obj = 0;
GLfloat window::move_speed = 4.0;
GLfloat window::border_width = 4.0;
rgbcolor window::border_col;
mouse window::mus;

// Executed when window system signals to shut down.
void window::close() {
   DEBUGF ('g', sys_info::execname() << ": exit ("
           << sys_info::exit_status() << ")");
   exit (sys_info::exit_status());
}

// Executed when mouse enters or leaves window.
void window::entry (int mouse_entered) {
   DEBUGF ('g', "mouse_entered=" << mouse_entered);
   window::mus.entered = mouse_entered;
   if (window::mus.entered == GLUT_ENTERED) {
      DEBUGF ('g', sys_info::execname() << ": width=" << window::width
           << ", height=" << window::height);
   }
   glutPostRedisplay();
}

// Called to display the objects in the window.
void window::display() {
   glClear (GL_COLOR_BUFFER_BIT);
   int num = 0;
   for (auto& object: window::objects)
   {
      object.draw();
      if (num <= 12)
         object.draw_number (num);
      num++;
   }
   GLfloat line_width;
   glGetFloatv (GL_LINE_WIDTH, &line_width);
   glLineWidth (window::border_width);
   if (selected_obj < objects.size())
   {
      objects[selected_obj].draw_outline (window::border_col);
   }
   glLineWidth (line_width);
   mus.draw();
   glutSwapBuffers();
}

// Called when window is opened and when resized.
void window::reshape (int width, int height) {
   DEBUGF ('g', "width=" << width << ", height=" << height);
   window::width = width;
   window::height = height;
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D (0, window::width, 0, window::height);
   glMatrixMode (GL_MODELVIEW);
   glViewport (0, 0, window::width, window::height);
   glClearColor (0.25, 0.25, 0.25, 1.0);
   glutPostRedisplay();
}


// Executed when a regular keyboard key is pressed.
enum {BS=8, TAB=9, ESC=27, SPACE=32, DEL=127};
void window::keyboard (GLubyte key, int x, int y) {
   DEBUGF ('g', "key=" << (unsigned)key << ", x=" << x << ", y=" << y);
   window::mus.set (x, y);
   switch (key) {
      case 'Q': case 'q': case ESC:
         window::close();
         break;
      case 'H': case 'h':
         objects.at (selected_obj).move (-move_speed, 0);
         break;
      case 'J': case 'j':
         objects.at (selected_obj).move (0, -move_speed);
         break;
      case 'K': case 'k':
         objects.at (selected_obj).move (0, move_speed);
         break;
      case 'L': case 'l':
         objects.at (selected_obj).move (move_speed, 0);
         break;
      case 'N': case 'n': case SPACE: case TAB:
         if (selected_obj + 1 < objects.size()) ++selected_obj;
         break;
      case 'P': case 'p': case BS:
         if (selected_obj > 0) --selected_obj;
         break;
      case '0'...'9':
      // Enclose in block to fix "crosses intialization" error
      {
         //select_object (key - '0');
         size_t num = key - '0';
         if (num < objects.size()) selected_obj = num;
         break;
      }
      default:
         cerr << (unsigned)key << ": invalid keystroke" << endl;
         break;
   }
   glutPostRedisplay();
}


// Executed when a special function key is pressed.
void window::special (int key, int x, int y) {
   DEBUGF ('g', "key=" << key << ", x=" << x << ", y=" << y);
   window::mus.set (x, y);
   switch (key) {
      case GLUT_KEY_LEFT: 
         objects.at (selected_obj).move (-move_speed, 0);
         break;
      case GLUT_KEY_DOWN: 
         objects.at (selected_obj).move (0, -move_speed);
         break;
      case GLUT_KEY_UP: 
         objects.at (selected_obj).move (0, move_speed);
         break;
      case GLUT_KEY_RIGHT:
         objects.at (selected_obj).move (move_speed, 0);
         break;
      case GLUT_KEY_F1: 
         if (1 < objects.size()) selected_obj = 1;
         break;
      case GLUT_KEY_F2:
         if (2 < objects.size()) selected_obj = 2;
         break;
      case GLUT_KEY_F3: 
         if (3 < objects.size()) selected_obj = 3;
         break;
      case GLUT_KEY_F4: 
         if (4 < objects.size()) selected_obj = 4;
         break;
      case GLUT_KEY_F5: 
         if (5 < objects.size()) selected_obj = 5;
         break;
      case GLUT_KEY_F6: 
         if (6 < objects.size()) selected_obj = 6;
         break;
      case GLUT_KEY_F7: 
         if (7 < objects.size()) selected_obj = 7;
         break;
      case GLUT_KEY_F8: 
         if (8 < objects.size()) selected_obj = 8;
         break;
      case GLUT_KEY_F9: 
         if (9 < objects.size()) selected_obj = 9;
         break;
      case GLUT_KEY_F10: 
         if (10 < objects.size()) selected_obj = 10;
         break;
      case GLUT_KEY_F11: 
         if (11 < objects.size()) selected_obj = 11;
         break;
      case GLUT_KEY_F12: 
         if (12 < objects.size()) selected_obj = 12;
         break;
      default:
         cerr << (unsigned)key << ": invalid function key" << endl;
         break;
   }
   glutPostRedisplay();
}


void window::motion (int x, int y) {
   DEBUGF ('g', "x=" << x << ", y=" << y);
   window::mus.set (x, y);
   glutPostRedisplay();
}

void window::passivemotion (int x, int y) {
   DEBUGF ('g', "x=" << x << ", y=" << y);
   window::mus.set (x, y);
   glutPostRedisplay();
}

void window::mousefn (int button, int state, int x, int y) {
   DEBUGF ('g', "button=" << button << ", state=" << state
           << ", x=" << x << ", y=" << y);
   window::mus.state (button, state);
   window::mus.set (x, y);
   glutPostRedisplay();
}

void window::main () {
   static int argc = 0;
   glutInit (&argc, nullptr);
   glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE);
   glutInitWindowSize (window::width, window::height);
   glutInitWindowPosition (128, 128);
   glutCreateWindow (sys_info::execname().c_str());
   glutCloseFunc (window::close);
   glutEntryFunc (window::entry);
   glutDisplayFunc (window::display);
   glutReshapeFunc (window::reshape);
   glutKeyboardFunc (window::keyboard);
   glutSpecialFunc (window::special);
   glutMotionFunc (window::motion);
   glutPassiveMotionFunc (window::passivemotion);
   glutMouseFunc (window::mousefn);
   DEBUGF ('g', "Calling glutMainLoop()");
   glutMainLoop();
}


void mouse::state (int button, int state) {
   switch (button) {
      case GLUT_LEFT_BUTTON: left_state = state; break;
      case GLUT_MIDDLE_BUTTON: middle_state = state; break;
      case GLUT_RIGHT_BUTTON: right_state = state; break;
   }
}

void mouse::draw() {
   static rgbcolor color ("green");
   ostringstream text;
   text << "(" << xpos << "," << window::height - ypos << ")";
   if (left_state == GLUT_DOWN) text << "L"; 
   if (middle_state == GLUT_DOWN) text << "M"; 
   if (right_state == GLUT_DOWN) text << "R"; 
   if (entered == GLUT_ENTERED) {
      void* font = GLUT_BITMAP_HELVETICA_18;
      glColor3ubv (color.ubvec);
      glRasterPos2i (10, 10);
      glutBitmapString (font, (GLubyte*) text.str().c_str());
   }
}

void object::move (GLfloat delta_x, GLfloat delta_y)
{
   center.xpos += delta_x;
   center.ypos += delta_y;
   if (center.xpos < 0) center.xpos = window::width;
   if (center.xpos > window::width) center.xpos = 0;
   if (center.ypos < 0) center.ypos = window::height;
   if (center.ypos > window::height) center.ypos = 0;
}

void object::draw_number(int num)
{
   uchar max_col = 255;
   rgbcolor col{max_col-color.red, max_col-color.green,
      max_col-color.blue};
   ostringstream text;
   text << num;
   void* font = GLUT_BITMAP_8_BY_13;
   glColor3ubv (col.ubvec);
   if (num < 10)
      glRasterPos2i (center.xpos - 4, center.ypos - 4);
   else
      glRasterPos2i (center.xpos - 6, center.ypos - 4);
   glutBitmapString (font, (GLubyte*) text.str().c_str());
}
