// Large amplitude pendulum:
// T = 2/pi*T0*ln(8/alpha_m)
// alpha is angle to balance
// T0 = 2pi sqrt(L/g)
// http://iopscience.iop.org/0143-0807/33/6/1555

#include "GL/glut.h"

#include "keys.h"

#include "Rope.h"
#include "method.h"

#include <cassert>
#include <cmath>
#include <deque>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

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

template<class T> T sqr(T x){return x*x;}

unsigned int gSallyTex, gRopeTex;

int gNumBells = 8; // Default in case of no argument passed
int gPealMins = 2*60+40;
int gMyBell = 0;
bool gAuto = false;
bool gGo = true;
Method* gMethod = 0;

GstElement** play = 0;
GMainLoop* loop;

std::map<std::pair<int, int>, double> training;

int sign(double x){return (x>=0) ? +1 : -1;}

class Bell
{
public:
  Bell() : fGone(false), fAngVel(0), fAngle(3), fHand(true), fPulling(false), fPlaying(false)
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
    // This is gravity, but the wheel radius and bell centre of mass are all
    // mixed in. I fiddled with it so that the bells naturally go at such a
    // speed that you can hunt out at 2h40 peal speed. The scaling with the
    // square of the peal speed is what the formulae for large amplitude
    // pendulums predicts, so should remain controllable at all speeds.
    const double g = 12*sqr(160./gPealMins);

    //    const double g = 1/sqr(.5*log(0.2/8)/4);

    if(fGone){
      fAngVel -= g*dt*sin(fAngle);

      if(fPulling){
	// Velocity that will cause the bell to come most of the way to the
	// balance
	const double target = sqrt(2*g*(cos(fAngle)-cos(3.05)));

	// Higher values make the bell respond more quickly to your pull, which
	// makes it easier. But eventually it'll be unrealistic.
	const double kPullForce = 3;

	if(fHand && fAngVel > -target){
	  fAngVel -= kPullForce*dt;
	}
	if(!fHand && fAngVel < +target){
	  fAngVel += kPullForce*dt;
	}
	if(fHand & fAngVel < -target){
	  fPulling = false;
	  fAngVel = -target;
	}
	if(!fHand & fAngVel > +target){
	  fPulling = false;
	  fAngVel = +target;
	}
      }

      fAngle += dt*fAngVel;

      if(fHand){
	// About to reach the bottom. Reset the sound so it'll be nice and ready to go again once we get there
	if(fAngle < 1 && fAngVel < 0 && fPlaying){
	  gst_element_set_state(play[fBell], GST_STATE_NULL);
	  gst_element_seek(play[fBell], 1, GST_FORMAT_DEFAULT, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, -1);
	  fPlaying = false;
	}

	// Bell strikes as it goes past the bottom, and you stop being able to pull at this stroke
	if(fAngle < 0){
	  fHand = false;
	  fPulling = false;
	  gst_element_set_state(play[fBell], GST_STATE_PLAYING);
	  fPlaying = true;
	}
      }
      else{
	if(fAngle > -1 && fAngVel > 0 && fPlaying){
	  gst_element_set_state(play[fBell], GST_STATE_NULL);
	  gst_element_seek(play[fBell], 1, GST_FORMAT_DEFAULT, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, -1);
	  fPlaying = false;
	}

	if(fAngle > 0){
	  fHand = true;
	  fPulling = false;
	  gst_element_set_state(play[fBell], GST_STATE_PLAYING);
	  fPlaying = true;
	}
      }
    }
  }
  void Pull()
  {
    fPulling = true;
  }
  bool Pulling() const {return fPulling;}
  double Angle() const {return fAngle;}
  double AngVel() const {return fAngVel;}
  double ExtraRope() const {return fabs(fAngle-1);}
protected:
  bool fGone;
  double fAngVel;
  double fAngle;
  int fBell;
  bool fHand;
  bool fPulling;
  bool fPlaying;
};

Bell* gBells;
//Rope* gRopes;
bool* pending;

long gStartTime;

double gLookAngle = 0;

int main(int argc, char** argv)
{
  glutInit(&argc, argv);

  gst_init(&argc, &argv);

  if(argc == 1) std::cout << "Usage: " << argv[0] << " numBells notation pealMins bell auto" << std::endl;

  std::string notation = "-";

  if(argc > 1) gNumBells = atoi(argv[1]);
  if(argc > 2) notation = argv[2];
  if(argc > 3) gPealMins = atoi(argv[3]);
  if(argc > 4) gMyBell = atoi(argv[4])-1;
  if(argc > 5) gAuto = (std::string(argv[5]) == "1");

  if(gMyBell == 0 && !gAuto) gGo = false; // If ringing treble, wait for me

  gMethod = new Method(gNumBells, notation);
  play = new GstElement*[gNumBells];
  gBells = new Bell[gNumBells];
  pending = new bool[gNumBells];
  for(int i = 0; i < gNumBells; ++i) pending[i] = false;
  //  gRopes = new Rope[gNumBells];

  FILE* f = fopen("train.txt", "r");
  while(!feof(f)){
    std::pair<int, int> key;
    double junk;
    double val;
    int ret = fscanf(f, "%d %d %lf %lf %lf", &key.first, &key.second, &junk, &junk, &val);
    if(val > 0) training[key] = val;
  }
  fclose(f);

  for(int i = 0; i < gNumBells; ++i){
    play[i] = gst_element_factory_make("playbin", "play");
    // Major scale: tone, tone, semitone, tone, tone, tone, semitone etc
    int notes[12] = {20, 18, 16, 15, 13, 11, 9, 8, 6, 4, 3, 1};
    char numstr[20];
    sprintf(numstr, "%d", notes[gNumBells-i-1]-notes[gNumBells-1]);
    char* cwd = getcwd(0, 0);
    g_object_set(G_OBJECT(play[i]), "uri", 
		 (std::string("file://")+cwd+std::string("/audio/tbell_")+numstr+".wav").c_str(), NULL);
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

  //  std::cout << "SALLY " << gSallyTex << std::endl;
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

  //  std::cout << "ROPE " << gRopeTex << std::endl;

  glBindTexture(GL_TEXTURE_2D, gRopeTex);
  glEnable(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, 3, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, texrope);


  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  glEnable(GL_CULL_FACE);

  glutFullScreen();

  glutMainLoop();

  for(int i = 0; i < gNumBells; ++i) gst_element_set_state(play[i], GST_STATE_NULL);
  gst_object_unref(GST_OBJECT(play));

  return 0;
}

void pullCallback(int bell)
{
  std::cout << "Pulling " << bell << std::endl;
  gBells[bell].Go(); // Just in case
  gBells[bell].Pull();
  pending[bell] = false;
}

#include <iostream>
using namespace std;
void OnIdle()
{
  while(g_main_context_iteration(0, false)){}

  if(!gGo) return; // Wait for treble to pull off

  long t = glutGet(GLUT_ELAPSED_TIME);

  // Allow for handstroke leads
  const int kPealTicks = gNumBells*5040+5040/2;

  const double tickLen = double(gPealMins*60*1000)/kPealTicks;

  static bool once = true;
  bool userPullOff = false;
  if(once){
    once = false;
    gStartTime = glutGet(GLUT_ELAPSED_TIME);
    LastUpdate = gStartTime;

    if(!gAuto && gMyBell == 0) userPullOff = true;
  }

  // Doesn't apply if user has control of the start
  if(gMyBell != 0 || gAuto){
    // Don't do anything in the first two seconds
    //    if(t < 2000) return;
    // Adjust clock to account for that
    //    t -= 2000;
  }

  double dt = (t-LastUpdate)/1000.0;

  // This will have bad consequences, but not as bad as letting the physics
  // engine take such a big time step.
  if(dt < 0 || dt > .2) return;

  const int tick = (t-gStartTime)/tickLen;

  static std::deque<double>* targets = 0;
  if(!targets){
    targets = new std::deque<double>[gNumBells];
  }

  static int lasttick = -1;//(LastUpdate-gStartTime)/tickLen;

  if(userPullOff){
    std::pair<int, int> key(fabs(gBells[0].Angle())/M_PI*1000,
			    (gBells[0].AngVel()*sign(gBells[0].Angle())/20+.5)*1000);
    for(int i = 1; i < gNumBells; ++i){
      targets[i].push_back((t+i*tickLen)/1000.+training[key]);
    }
    // This makes everything go wrong. I don't know why. Just let this row be
    // put in twice and recover from that.
    // lasttick = gNumBells-1;
  }

  while(tick > lasttick){
    double late = (t-gStartTime-tick*tickLen)/1000.;
    ++lasttick;
    const int bell = gMethod->BellAt(tick);
    if(bell >= 0){ // Ignore handstroke leads
      if(bell != gMyBell || gAuto){ // Don't ring user's bell
	targets[bell].push_back(t/1000.-late+3);
	//	std::cout << "Setting target " << bell << " " << t/1000.-late+1.2 << std::endl;
      }
    }
  }
  
  for(int bell = 0; bell < gNumBells; ++bell){
    std::pair<int, int> key(fabs(gBells[bell].Angle())/M_PI*1000,
			    (gBells[bell].AngVel()*sign(gBells[bell].Angle())/20+.5)*1000);
    
    //    std::cout << key.first << " " << key.second << std::endl;
    assert(training.find(key) != training.end());

    if(!targets[bell].empty() &&
       (t/1000.+training[key] > targets[bell].front())){
      //       std::cout << t/1000. << " Pulling " << bell << " " << training[key] << std::endl;

      const double estLate = (t/1000.+training[key] - targets[bell].front());

      if(estLate > .1){
	std::cout << "Uh oh. Bell " << bell+1 << " expects to be " << estLate << "s late" << std::endl;
	std::cout << "  Because its target is " <<  targets[bell].front() << " and it is now " << t/1000. << " with an estimated time to strike of " << training[key] << std::endl;
      }

      gBells[bell].Go(); // Just in case
      gBells[bell].Pull();
      targets[bell].pop_front();
    }
  }

  /*
	if(!pending[bell]){
	// Don't bother to pull twice
	//	if(!gBells[bell].Pulling()){

	std::cout << t << ": Will pull " << bell << " in " << (1.2-training[key]-late) << std::endl;
	pending[bell] = true;
	glutTimerFunc((1.2-training[key]-late)*1000, pullCallback, bell);
	}
  */

	/*
	  //	std::cout << key << std::endl;
	  //	std::cout << training[key] << std::endl;
	  if(training[key] < 0 || training[key]+late > 1.2){
	    std::cout << bell << " " << training[key] << " " << late << " " << training[key]+late << std::endl;
	    gBells[bell].Go(); // Just in case
	    gBells[bell].Pull();
	  }
	}
	*/
  //      }
  //    }
  //}

  LastUpdate = t;

  for(int i = 0; i < gNumBells; ++i)
    gBells[i].Update(dt);

  //  for(int i = 0; i < gNumBells; ++i)
  //    gRopes[i].Update(dt);

  if(keys.left) gLookAngle -= dt;
  if(keys.right) gLookAngle += dt;

  glutPostRedisplay();

  //  cout << int(1/dt) << " FPS\r" << flush;

  while(g_main_context_iteration(0, false)){}
}

void OnSize(int x, int y)
{
  glViewport(0, 0, x, y);
  glDrawBuffer(GL_BACK);

  // Must be more than kCameraDistance+kCircleRadius
  const double kMaxDistance = 1000;

  glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, x/(y+1.0), 1, kMaxDistance);
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

  const int kCylinderDetail = 20;

  // Units are cm

  // Have to stand crazy far back to see everything clearly. Eyes: how do they
  // work?
  const double kCameraDistance = 450;
  // Have to stand aside slightly to see the rope on the other side of the
  // circle
  const double kAsideDistance = 20;

  const double kCircleRadius = 200;//20;
  const double kSallyHeight = 80;
  const double kSallyEndHeight = 10;//2;

  const double kSallyRadius = 5;
  const double kRopeRadius = 1;//.2;

  const double kSallyBaseHeight = -400;
  const double kTailEndLength = 200;

  const double kRopeTravel = 175; // Related to wheel radius

  gluLookAt(kCameraDistance, kAsideDistance, 0, 0/*40-cos(gLookAngle)*/, kCameraDistance*sin(gLookAngle), 0, 0, 0, 1);

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

  //  glTranslated(0, 0, -5);

  for(int n = 0; n < gNumBells; ++n){

    glPushMatrix();

    glRotated(-(n-gMyBell)*360./gNumBells, 0, 0, 1);
    glTranslated(kCircleRadius, 0, 0);

    // Rotate back so that the "seam" on the sally is always in the same
    // place. Away from the camera.
    glRotated(+(n-gMyBell)*360./gNumBells+90, 0, 0, 1);

    //    glBindTexture(GL_TEXTURE_2D, gRopeTex);
    //    glEnable(GL_TEXTURE_2D);
    //    glDisable(GL_TEXTURE_2D);

    //    glTranslated(0, 0, -kRopeMaxHeight+20*gBells[n].ExtraRope());

    //    gRopes[n].SetTop(XYZ(0, 0, -kRopeMaxHeight+20*gBells[n].ExtraRope()));
    //    gRopes[n].SetBottom(XYZ(2, 0, -40+20*gBells[n].ExtraRope()+5));
    //    gRopes[n].Draw();

    glBindTexture(GL_TEXTURE_2D, gSallyTex);
    glEnable(GL_TEXTURE_2D);

    glTranslated(0, 0, kSallyBaseHeight+kRopeTravel*gBells[n].ExtraRope());

    glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glScaled(1, 5, 1);
      glRotated(-30, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW); 

    // Main sally piece
    GLUquadric* q = gluNewQuadric();
    gluQuadricTexture(q, GLU_TRUE);
    //    glColor3d(.5, 0, .5);
    glColor3d(1, 1, 1);
    gluCylinder(q, kSallyRadius, kSallyRadius, kSallyHeight, kCylinderDetail, kCylinderDetail);

    // Tops and bottoms

    glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glScaled(1, 1, 1);
      glRotated(-45, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW); 

    // Ends of sally
    glTranslated(0, 0, kSallyHeight);
    gluCylinder(q, kSallyRadius, 0, kSallyEndHeight, kCylinderDetail, kCylinderDetail);
    glTranslated(0, 0, -kSallyHeight-kSallyEndHeight);
    gluCylinder(q, 0, kSallyRadius, kSallyEndHeight, kCylinderDetail, kCylinderDetail);

    // Don't texture rope since we can't make it work right yet
    glDisable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, gRopeTex);
    //    glEnable(GL_TEXTURE_2D);

    /*
    glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      //      glScaled(1, 20, 1);
      //      glRotated(-20, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW); 
    */

    // Rope
    glTranslated(0, 0, -kTailEndLength);
    glColor3d(.75, .75, 0);
    gluCylinder(q, kRopeRadius, kRopeRadius, 1000, kCylinderDetail, kCylinderDetail);

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
  // Key up
  if(!gAuto && state == false && key == ' '){
    gBells[gMyBell].Go(); // Just in case
    gBells[gMyBell].Pull();
    if(gMyBell == 0) gGo = true;
  }

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
  gLookAngle = .00025*(x-1920/2);
}
