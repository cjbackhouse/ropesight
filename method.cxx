#include "method.h"

#include <iostream>

Method::Method(int nbells, const std::string& notation)
  : fTick(0), fRowPos(0), fNotationPos(0),
    fNotation(notation)
{
  for(int n = 0; n < nbells; ++n) fRow.push_back(n);
}

int Method::BellAt(int tick)
{
  if(tick <= fTick){
    //    std::cout << tick << " " << fRow[fRowPos] << std::endl;
    return fRow[fRowPos];
  }

  ++fRowPos;
  ++fTick;
  if(fRowPos == fRow.size()){
    fRowPos = 0;
    if(fNotation != "-"){
      if(fNotation[fNotationPos] == 'x'){
	for(int i = 0; i < fRow.size()-2; i += 2) std::swap(fRow[i], fRow[i+1]);
	++fNotationPos;
      }
      else{ // not an x
	for(int i = 0; i < fRow.size()-2; i += 2){
	  while(fNotationPos < fNotation.size() && CharToBell(fNotation[fNotationPos]) == i){
	    ++fNotationPos;
	    ++i;
	  }
	  std::swap(fRow[i], fRow[i+1]);
	}
	// Eat trailing numbers that were superfluous
	while(fNotationPos < fNotation.size() && CharToBell(fNotation[fNotationPos] < 0)) ++fNotationPos;
	// Eat a dot if there is one
	if(fNotationPos < fNotation.size() && fNotation[fNotationPos] == '.') ++fNotationPos;
      }
      if(fNotationPos == fNotation.size()) fNotationPos = 0;
    }
  }
  //  std::cout << tick << " " << fRow[fRowPos] << std::endl;
  return fRow[fRowPos];
}

int Method::CharToBell(char chr) const
{
  if(chr == '0') return 9;
  if(chr == 'E') return 10;
  if(chr == 'T') return 11;
  if(chr >= '1' && chr <= '9') return chr-'1';
  return -1; // Dots and so on
}
