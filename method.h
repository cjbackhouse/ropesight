#ifndef METHOD_H
#define METHOD_H

#include <string>
#include <vector>

class Method
{
public:
  Method(int nbells, const std::string& notation);
  int BellAt(int tick);
protected:
  int CharToBell(char chr) const;

  int fTick;
  int fRowPos;
  int fNotationPos;
  std::vector<int> fRow;
  std::string fNotation;
};

#endif
