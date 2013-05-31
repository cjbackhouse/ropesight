#include <cmath>

#ifndef VECTORNDHEADER
#define VECTORNDHEADER

class Vector2D
{
public:
  Vector2D(): x(0),y(0){}
  Vector2D(float a,float b): x(a),y(b){}
  Vector2D operator*(float m) const{Vector2D ret=*this;ret*=m;return ret;}
  Vector2D& operator*=(float m){x*=m;y*=m;return *this;}
  Vector2D operator-(Vector2D a) const{Vector2D ret=*this;ret-=a;return ret;}
  Vector2D operator+(Vector2D a) const{Vector2D ret=*this;ret+=a;return ret;}
  Vector2D& operator+=(Vector2D a){x+=a.x;y+=a.y;return *this;}
  Vector2D& operator-=(Vector2D a){x-=a.x;y-=a.y;return *this;}
  Vector2D operator/(float m) const{Vector2D ret=*this;ret/=m;return ret;}
  Vector2D& operator/=(float m){x/=m;y/=m;return *this;}
  float x,y;
};
float MagSq(Vector2D a);
float Magnitude(Vector2D a);
float DotProduct(Vector2D a,Vector2D b);
float CrossProduct(Vector2D a,Vector2D b);
Vector2D Normalise(Vector2D& v);

typedef Vector2D XY;


class Vector3D
{
public:
  Vector3D():x(0),y(0),z(0){}
  Vector3D(float a,float b,float c): x(a),y(b),z(c){}
  Vector3D operator*(float m) const{Vector3D ret=*this;ret*=m;return ret;}
  Vector3D& operator*=(float m){x*=m;y*=m;z*=m;return *this;}
  Vector3D operator-(const Vector3D& a) const{Vector3D ret=*this;ret-=a;return ret;}
  Vector3D operator+(const Vector3D& a) const{Vector3D ret=*this;ret+=a;return ret;}
  Vector3D& operator+=(const Vector3D& a){x+=a.x;y+=a.y;z+=a.z;return *this;}
  Vector3D& operator-=(const Vector3D& a){x-=a.x;y-=a.y;z-=a.z;return *this;}
  Vector3D operator/(float m) const{Vector3D ret=*this;ret/=m;return ret;}
  Vector3D& operator/=(float m){x/=m;y/=m;z/=m;return *this;}
  float x,y,z;
};
float MagSq(const Vector3D& a);
float Magnitude(const Vector3D& a);
float DotProduct(const Vector3D& a, const Vector3D& b);
Vector3D CrossProduct(Vector3D a,Vector3D b);
Vector3D Normalised(const Vector3D& v);

typedef Vector3D XYZ;


/*
   Rotate a point p by angle theta around an arbitrary axis r
   Return the rotated point.
   Positive angles are anticlockwise looking down the axis
   towards the origin.
   Assume right hand coordinate system.
*/
XYZ ArbitraryRotate(XYZ p,float theta,XYZ r);


/*
   Rotate a point p by angle theta around an arbitrary line segment p1-p2
   Return the rotated point.
   Positive angles are anticlockwise looking down the axis
   towards the origin.
   Assume right hand coordinate system.
*/
XYZ ArbitraryRotate2(XYZ p,float theta,XYZ p1,XYZ p2);

#endif
