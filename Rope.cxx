#include "Rope.h"

#include "GL/glut.h"

const double kSegLen = .2;
const int kNumSegs = 30;
const double kSpringConst = 1000;
const double kFriction = 1;
const double kInternalFriction = 10;
const double kStiffness = 10;

const double kGravity = 100;//9.81;

Rope::Rope()
{
  fPos.resize(kNumSegs);
  fVel.resize(kNumSegs);

  for(int n = 0; n < kNumSegs; ++n){
    //    fPos[n] = XYZ(2, 2+1*sin(.25*n), 5-n*kSegLen);

    //    fPos[n] = XYZ(2, 2+kSegLen/M_PI*sin(M_PI*n/double(kNumSegs)), 5+kSegLen/M_PI*(1-cos(M_PI*n/double(kNumSegs))));

    fPos[n] = XYZ(sin(n/double(kNumSegs)), cos(n/double(kNumSegs)), -n*kSegLen);
  }
}

void Rope::Update(double dt)
{
  // TODO: Runge Kutta method instead
  dt /= 100;
  for(int i = 0; i < 100; ++i){


  fPos[0] = fTop;//XYZ(2, 2, 5);
  fVel[0] = XYZ(0, 0, 0); // TODO should derive

  //  fPos[kNumSegs-1] = fBottom;
  //  fVel[kNumSegs-1] = XYZ(0, 0, 0); // TODO should derive

  for(int n = 0; n < kNumSegs; ++n){
    fVel[n].z -= kGravity*dt;

    XYZ fric = Normalised(fVel[n]);
    fVel[n] -= fric*kFriction*dt;
  }

  for(int n = 0; n+1 < kNumSegs; ++n){
    XYZ diff = fPos[n+1]-fPos[n];
    const double d = Magnitude(diff);
    diff = Normalised(diff);
    fVel[n]   += diff*kSpringConst*(d-kSegLen)*dt;
    fVel[n+1] -= diff*kSpringConst*(d-kSegLen)*dt;
  }

  for(int n = 0; n+1 < kNumSegs; ++n){
    XYZ diff = fVel[n+1]-fVel[n];
    fVel[n]   += diff*kInternalFriction*dt;
    fVel[n+1] -= diff*kInternalFriction*dt;
  }

  // Potential is dr_i dot dr_i+1
  for(int n = 1; n+1 < kNumSegs; ++n){
    XYZ diff0 = Normalised(fPos[n+1]-fPos[n]);
    XYZ diff1 = Normalised(fPos[n]-fPos[n-1]);

    fVel[n] -= (diff0-diff1)*kStiffness*dt;
  }

  for(int n = 0; n < kNumSegs; ++n){
    fPos[n] += fVel[n]*dt;
  }

  fPos[0] = fTop;//XYZ(2, 2, 5);
  fVel[0] = XYZ(0, 0, 0); // TODO: should actually derive this

  //  fPos[kNumSegs-1] = fBottom;
  //  fVel[kNumSegs-1] = XYZ(0, 0, 0); // TODO should derive

  }
}

void Rope::Draw()
{
  glColor3d(.75, .75, 0);
  glBegin(GL_LINE_STRIP);
  for(int n = 0; n < kNumSegs; ++n){
    glVertex3d(fPos[n].x, fPos[n].y, fPos[n].z);
  }
  glEnd();

  for(int angIdx = 0; angIdx < 10; ++angIdx){
    double ang0 = (angIdx*M_PI*2)/10;
    double ang1 = ((angIdx+1)*M_PI*2)/10;
    glBegin(GL_QUAD_STRIP);
    for(int n = 0; n < kNumSegs; ++n){
      XYZ diff = n ? (fPos[n]-fPos[n-1]) : (fPos[n+1]-fPos[n]);
      XYZ dx = CrossProduct(diff, XYZ(1, 0, 0));
      XYZ dy = CrossProduct(diff, dx);
      dx = Normalised(dx);
      dy = Normalised(dy);

      XYZ da = dx*cos(ang0)+dy*sin(ang0);
      XYZ db = dx*cos(ang1)+dy*sin(ang1);

      XYZ a = fPos[n]+da*.2;
      XYZ b = fPos[n]+db*.2;

      glTexCoord2d(ang0/(2*M_PI), n);
      glNormal3d(da.x, da.y, da.z);
      glVertex3d(a.x, a.y, a.z);

      glTexCoord2d(ang1/(2*M_PI), n);
      glNormal3d(db.x, db.y, db.z);
      glVertex3d(b.x, b.y, b.z);
    }
    glEnd();
  } // angIdx
}
