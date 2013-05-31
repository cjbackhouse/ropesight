#include <cmath>
#include "XYZ.h"

float MagSq(Vector2D a){return a.x*a.x+a.y*a.y;}
float Magnitude(Vector2D a){return sqrt(MagSq(a));}
float DotProduct(Vector2D a,Vector2D b){return a.x*b.x+a.y*b.y;}
float CrossProduct(Vector2D a,Vector2D b){return a.x*b.y-a.y*b.x;}
Vector2D Normalise(Vector2D& v){return v/=Magnitude(v);}

typedef Vector2D XY;

float MagSq(const Vector3D& a){return a.x*a.x+a.y*a.y+a.z*a.z;}
float Magnitude(const Vector3D& a){return sqrt(MagSq(a));}
float DotProduct(const Vector3D& a,const Vector3D& b){return a.x*b.x+a.y*b.y+a.z*b.z;}
Vector3D CrossProduct(Vector3D a,Vector3D b){return Vector3D(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x);}
Vector3D Normalised(const Vector3D& v){return v/Magnitude(v);}

typedef Vector3D XYZ;


/*
   Rotate a point p by angle theta around an arbitrary axis r
   Return the rotated point.
   Positive angles are anticlockwise looking down the axis
   towards the origin.
   Assume right hand coordinate system.
*/
XYZ ArbitraryRotate(XYZ p,float theta,XYZ r)
{
   Vector3D q;
   float costheta,sintheta;

   r = Normalised(r);
   costheta = cos(theta);
   sintheta = sin(theta);

   q.x += (costheta + (1 - costheta) * r.x * r.x) * p.x;
   q.x += ((1 - costheta) * r.x * r.y - r.z * sintheta) * p.y;
   q.x += ((1 - costheta) * r.x * r.z + r.y * sintheta) * p.z;

   q.y += ((1 - costheta) * r.x * r.y + r.z * sintheta) * p.x;
   q.y += (costheta + (1 - costheta) * r.y * r.y) * p.y;
   q.y += ((1 - costheta) * r.y * r.z - r.x * sintheta) * p.z;

   q.z += ((1 - costheta) * r.x * r.z - r.y * sintheta) * p.x;
   q.z += ((1 - costheta) * r.y * r.z + r.x * sintheta) * p.y;
   q.z += (costheta + (1 - costheta) * r.z * r.z) * p.z;

   return(q);
}

/*
   Rotate a point p by angle theta around an arbitrary line segment p1-p2
   Return the rotated point.
   Positive angles are anticlockwise looking down the axis
   towards the origin.
   Assume right hand coordinate system.
*/
XYZ ArbitraryRotate2(XYZ p,float theta,XYZ p1,XYZ p2)
{
   Vector3D q;
   float costheta,sintheta;
   XYZ r;

   r.x = p2.x - p1.x;
   r.y = p2.y - p1.y;
   r.z = p2.z - p1.z;
   p.x -= p1.x;
   p.y -= p1.y;
   p.z -= p1.z;
   r = Normalised(r);

   costheta = cos(theta);
   sintheta = sin(theta);

   q.x += (costheta + (1 - costheta) * r.x * r.x) * p.x;
   q.x += ((1 - costheta) * r.x * r.y - r.z * sintheta) * p.y;
   q.x += ((1 - costheta) * r.x * r.z + r.y * sintheta) * p.z;

   q.y += ((1 - costheta) * r.x * r.y + r.z * sintheta) * p.x;
   q.y += (costheta + (1 - costheta) * r.y * r.y) * p.y;
   q.y += ((1 - costheta) * r.y * r.z - r.x * sintheta) * p.z;

   q.z += ((1 - costheta) * r.x * r.z - r.y * sintheta) * p.x;
   q.z += ((1 - costheta) * r.y * r.z + r.x * sintheta) * p.y;
   q.z += (costheta + (1 - costheta) * r.z * r.z) * p.z;

   q.x += p1.x;
   q.y += p1.y;
   q.z += p1.z;
   return(q);
}
