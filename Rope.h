#pragma once

#include "XYZ.h"

#include <vector>

class Rope
{
public:
  Rope();
  void Update(double dt);
  void Draw();

  void SetTop(XYZ t) {fTop = t;}
  void SetBottom(XYZ b) {fBottom = b;}
protected:
  XYZ fTop;
  XYZ fBottom;
  std::vector<XYZ> fPos;
  std::vector<XYZ> fVel;
};
