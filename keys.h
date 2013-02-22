#ifndef KEYS_HEADER
#define KEYS_HEADER

struct Keys
{
  Keys(){up = down = left = right = z = x = space = false;}
  bool up, down, left, right, z, x, space;
};

#endif
