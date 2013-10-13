#include "GL/glut.h"

#include "keys.h"

#include "Rope.h"

#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <gst/gst.h>

Keys keys;

long LastUpdate;
void OnIdle();
void OnDraw();
void OnSize(int, int);
void OnKeyDn(int, int, int);
void OnKeyUp(int, int, int);
void OnCharDn(unsigned char, int, int);
void OnCharUp(unsigned char, int, int);
void OnMotion(int, int);

unsigned int gSallyTex, gRopeTex;

GstElement* play[12] = {0,};
GMainLoop* loop;

class Bell
{
public:
  Bell() : fGone(false), fAngVel(0), fAngle(3), fHand(true), fPlaying(false)
  {
    // Gross hack to assign them correctly
    static int i = 0;
    fBell = i;
    ++i;
  }
  void Go()
  {
    if(!fGone) fGone = true;
  }
  void Update(double dt)
  {
    if(fGone){
      fAngVel -= 4*dt*sin(fAngle);
      fAngle += dt*fAngVel;

      if(fHand){
	if(fAngle < 0){
	  fHand = false;
	  gst_element_set_state(play[fBell], GST_STATE_PLAYING);
	  fPlaying = true;
	}
      }
      else{
	if(fAngle > 0){
	  fHand = true;
	  gst_element_set_state(play[fBell], GST_STATE_PLAYING);
	  fPlaying = true;
	}
      }

      // Try and "preload" the seeking
      if(fPlaying && fabs(fAngle) > 2){
	gst_element_set_state(play[fBell], GST_STATE_NULL);
	gst_element_seek(play[fBell], 1, GST_FORMAT_DEFAULT, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, -1);
	fPlaying = false;
      }
    }
  }
  double Angle() const {return fAngle;}
  double ExtraRope() const {return fabs(fAngle-1);}
protected:
  bool fGone;
  double fAngVel;
  double fAngle;
  int fBell;
  bool fHand;
  bool fPlaying;
};

Bell gBells[12];
Rope gRopes[12];

long gStartTime;

double gLookAngle = 0;

int main(int argc, char** argv)
{
  glutInit(&argc, argv);

  gst_init(&argc, &argv);
  for(int i = 0; i < 12; ++i){
    play[i] = gst_element_factory_make("playbin", "play");
    char numstr[20];
    sprintf(numstr, "%d", i);
    g_object_set(G_OBJECT(play[i]), "uri", 
		 (std::string("file:///home/bckhouse/Projects/Ropesight/audio/tbell_")+numstr+".wav").c_str(), NULL);
  }
  loop = g_main_loop_new(0, false);

  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(800, 600);
  glutCreateWindow("RopeSight");
  glutSetCursor(GLUT_CURSOR_CROSSHAIR);

  glutReshapeFunc(OnSize);
  glutDisplayFunc(OnDraw);
  glutSpecialFunc(OnKeyDn);
  glutSpecialUpFunc(OnKeyUp);
  glutKeyboardFunc(OnCharDn);
  glutKeyboardUpFunc(OnCharUp);
//  glutMouseFunc(OnClick);
//  glutMotionFunc(OnDrag);
  glutPassiveMotionFunc(OnMotion);

  glutIdleFunc(OnIdle);

  unsigned char tex[128][128][3];
  for(int x = 0; x < 128; ++x){
    for(int y = 0; y < 128; ++y){
      if(y < 128/3){
	tex[y][x][0] = 255-rand()%128;
	tex[y][x][1] = 0;
	tex[y][x][2] = 0;
      }
      else if(y > (2*128)/3){
	tex[y][x][0] = 0;
	tex[y][x][1] = 0;
	tex[y][x][2] = 255-rand()%128;
      }
      else{
	const int r = rand()%128;
	tex[y][x][0] = 255-r;
	tex[y][x][1] = 255-r;
	tex[y][x][2] = 255-r;
      }
    }
  }

  glGenTextures(1, &gSallyTex);
  glGenTextures(1, &gRopeTex);

  std::cout << "SALLY " << gSallyTex << std::endl;
  glBindTexture(GL_TEXTURE_2D, gSallyTex);

  glEnable(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, 3, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, tex);



  unsigned char texrope[128][128][3];
  for(int x = 0; x < 128; ++x){
    for(int y = 0; y < 128; ++y){
      if(y < 128/4){
	texrope[y][x][0] = 0;
	texrope[y][x][1] = 0;
	texrope[y][x][2] = 0;
      }
      else{
	texrope[y][x][0] = 255;
	texrope[y][x][1] = 255;
	texrope[y][x][2] = 255;
      }
    }
  }

  std::cout << "ROPE " << gRopeTex << std::endl;

  glBindTexture(GL_TEXTURE_2D, gRopeTex);
  glEnable(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, 3, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, texrope);


  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  glEnable(GL_CULL_FACE);

  glutFullScreen();

  glutMainLoop();

  for(int i = 0; i < 12; ++i) gst_element_set_state(play[i], GST_STATE_NULL);
  gst_object_unref(GST_OBJECT(play));

  return 0;
}


#include <iostream>
using namespace std;
void OnIdle()
{
  while(g_main_context_iteration(0, false)){}

  static bool once = true;
  if(once){
    once = false;
    gStartTime = glutGet(GLUT_ELAPSED_TIME);
  }

  long t = glutGet(GLUT_ELAPSED_TIME);
  double dt = (t-LastUpdate)/1000.0;
  LastUpdate = t;

  for(int i = 0; i < 12; ++i){
    if(t-gStartTime > 200*i) gBells[i].Go();
  }

  if(dt < 0 || dt > 1) return;

  for(int i = 0; i < 12; ++i)
    gBells[i].Update(dt);

  for(int i = 0; i < 12; ++i)
    gRopes[i].Update(dt);

  if(keys.left) gLookAngle -= dt;
  if(keys.right) gLookAngle += dt;

  glutPostRedisplay();

  cout << int(1/dt) << " FPS\r" << flush;

  while(g_main_context_iteration(0, false)){}
}

void OnSize(int x, int y)
{
  glViewport(0, 0, x, y);
  glDrawBuffer(GL_BACK);

  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, x/(y+1.0), 1, 512);
  glMatrixMode(GL_MODELVIEW);
}

void OnDraw()
{
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  float p[4] = {40, 10, 10, 1};
  glLightfv(GL_LIGHT0, GL_POSITION, p);
  glEnable(GL_LIGHT0);

  glClearColor(.5, .75, 1, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();

  //  gluLookAt(40, 0, 0, 40-cos(gLookAngle), sin(gLookAngle), 0, 0, 0, 1);

  gluLookAt(100, 0, 0, 40-cos(gLookAngle), sin(gLookAngle), 0, 0, 0, 1);

  /*
  glColor3d(.5, .5, .5);
  glBegin(GL_QUADS);
    // Floor
    glNormal3d(0, 0, 1);
    glVertex3d(-30, -30, -50);
    glVertex3d(+30, -30, -50);
    glVertex3d(+30, +30, -50);
    glVertex3d(-30, +30, -50);

    // Back wall
    glNormal3d(1, 0, 0);
    glVertex3d(-30, -30, -50);
    glVertex3d(-30, +30, -50);
    glVertex3d(-30, +30, +50);
    glVertex3d(-30, -30, +50);

    // Left wall
    glNormal3d(0, 1, 0);
    glVertex3d(-30, -30, -50);
    glVertex3d(-30, -30, +50);
    glVertex3d(+30, -30, +50);
    glVertex3d(+30, -30, -50);

    // Right wall
    glNormal3d(0, -1, 0);
    glVertex3d(-30, +30, -50);
    glVertex3d(+30, +30, -50);
    glVertex3d(+30, +30, +50);
    glVertex3d(-30, +30, +50);

    // Ceiling
    glNormal3d(0, 0, -1);
    glVertex3d(-30, -30, +50);
    glVertex3d(-30, +30, +50);
    glVertex3d(+30, +30, +50);
    glVertex3d(+30, -30, +50);
  glEnd();
  */

  glTranslated(0, 0, -5);

  for(int n = 0; n < 12; ++n){

    glPushMatrix();

    glRotated(-n*30, 0, 0, 1);
    glTranslated(20, 0, 0);

    // Rotate back so that the "seam" on the sally is always in the same
    // place. Away from the camera.
    glRotated(+n*30, 0, 0, 1);

    glBindTexture(GL_TEXTURE_2D, gRopeTex);
    glEnable(GL_TEXTURE_2D);

    glTranslated(0, 0, -40+20*gBells[n].ExtraRope());
    gRopes[n].SetTop(XYZ(0, 0, -40+20*gBells[n].ExtraRope()));
    //    gRopes[n].SetBottom(XYZ(2, 0, -40+20*gBells[n].ExtraRope()+5));
    //    gRopes[n].Draw();

    glBindTexture(GL_TEXTURE_2D, gSallyTex);
    glEnable(GL_TEXTURE_2D);

    glTranslated(0, 0, -40+20*gBells[n].ExtraRope());

    glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glScaled(1, 5, 1);
      glRotated(-30, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW); 

    GLUquadric* q = gluNewQuadric();
    gluQuadricTexture(q, GLU_TRUE);
    //    glColor3d(.5, 0, .5);
    glColor3d(1, 1, 1);
    gluCylinder(q, 1, 1, 10, 20, 20);

    // Tops and bottoms

    glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glScaled(1, 1, 1);
      glRotated(-45, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW); 

    glTranslated(0, 0, 10);
    gluCylinder(q, 1, 0, 2, 20, 20);
    glTranslated(0, 0, -12);
    gluCylinder(q, 0, 1, 2, 20, 20);

    // Rope
    //    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gRopeTex);
    glEnable(GL_TEXTURE_2D);

    /*
    glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      //      glScaled(1, 20, 1);
      //      glRotated(-20, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW); 
    */

    glTranslated(0, 0, 5); // get inside sally
    glColor3d(.75, .75, 0);
    gluCylinder(q, .2, .2, 80, 20, 20);

    glTranslated(0, 0, -40);
    glColor3d(.75, .75, 0);
    gluCylinder(q, .2, .2, 100, 20, 40);

    gluDeleteQuadric(q);

    glPopMatrix();
  }

  //  glDisable(GL_TEXTURE_2D);
  //  glDisable(GL_CULL_FACE);
  //  gRope.Draw();

  glutSwapBuffers();
}

void OnKey(int key, bool state)
{
  switch(key)
  {
  case(GLUT_KEY_UP):    keys.up    = state; break;
  case(GLUT_KEY_DOWN):  keys.down  = state; break;
  case(GLUT_KEY_LEFT):  keys.left  = state; break;
  case(GLUT_KEY_RIGHT): keys.right = state; break;
  case('x'): keys.x     = state; break;
  case('z'): keys.z     = state; break;
  case(' '): keys.space = state; break;
  case('q'): exit(0);
  }
}

void OnKeyDn(int k, int x, int y){OnKey(k, true);}
void OnKeyUp(int k, int x, int y){OnKey(k, false);}
void OnCharDn(unsigned char k, int x, int y){OnKey(k, true);}
void OnCharUp(unsigned char k, int x, int y){OnKey(k, false);}

void OnMotion(int x, int y)
{
  gLookAngle = .0005*(x-1920/2);
}
