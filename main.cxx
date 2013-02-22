#include "GL/glut.h"

#include "keys.h"

#include <cmath>
#include <iostream>

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

class Bell
{
public:
  Bell() : fGone(false), fAngVel(0), fAngle(3) {}
  void Go()
  {
    if(!fGone) fGone = true;
  }
  void Update(double dt)
  {
    if(fGone){
      fAngVel -= 2*dt*sin(fAngle);
      fAngle += dt*fAngVel;
    }
  }
  double Angle() const {return fAngle;}
  double ExtraRope() const {return fabs(fAngle-1);}
protected:
  bool fGone;
  double fAngVel;
  double fAngle;
};

Bell gBells[12];

long gStartTime;

double gLookAngle = 0;

int main(int argc, char** argv)
{
  glutInit(&argc, argv);

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
	tex[y][x][0] = 255;
	tex[y][x][1] = 0;
	tex[y][x][2] = 0;
      }
      else if(y > (2*128)/3){
	tex[y][x][0] = 0;
	tex[y][x][1] = 0;
	tex[y][x][2] = 255;
      }
      else{
	tex[y][x][0] = 255;
	tex[y][x][1] = 255;
	tex[y][x][2] = 255;
      }
    }
  }

  glGenTextures(1, &gSallyTex);
  std::cout << "SALLY " << gSallyTex << std::endl;
  glBindTexture(GL_TEXTURE_2D, gSallyTex);

  glEnable(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, 3, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, tex);



  unsigned char texrope[128][128][3];
  for(int x = 0; x < 128; ++x){
    for(int y = 0; y < 128; ++y){
      if(y < 128/2){
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

  glGenTextures(1, &gRopeTex);
  std::cout << "ROPE " << gRopeTex << std::endl;

  glEnable(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, gRopeTex);

  glTexImage2D(GL_TEXTURE_2D, 0, 3, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, texrope);


  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  glEnable(GL_CULL_FACE);

  glutFullScreen();

  glutMainLoop();
  return 0;
}


#include <iostream>
using namespace std;
void OnIdle()
{
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

  if(keys.left) gLookAngle -= dt;
  if(keys.right) gLookAngle += dt;

  glutPostRedisplay();

  cout << int(1/dt) << " FPS\r" << flush;
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

  glClearColor(.75, .75, .75, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();

  //  gluLookAt(20, 20, 20, 0, 0, 0, 0, 0, 1);
  gluLookAt(40, 0, 0, 40-cos(gLookAngle), sin(gLookAngle), 0, 0, 0, 1);

  glTranslated(0, 0, -5);

  for(int n = 0; n < 12; ++n){

    glPushMatrix();

    glRotated(-n*30, 0, 0, 1);
    glTranslated(20, 0, 0);

    glTranslated(0, 0, -20+10*gBells[n].ExtraRope());

    glBindTexture(GL_TEXTURE_2D, gSallyTex);
    glEnable(GL_TEXTURE_2D);

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
      //      glRotated(-30, 0, 0, 1);
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

    glTranslated(0, 0, -20);
    glColor3d(.75, .75, 0);
    gluCylinder(q, .2, .2, 100, 20, 20);
    gluDeleteQuadric(q);

    glPopMatrix();
  }
 

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
