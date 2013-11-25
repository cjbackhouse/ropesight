#include "method.h"

#include <iostream>

Method::Method(int nbells, const std::string& notation)
  : fTick(0), fRowPos(0), fNotationPos(0),
    fNotation(notation)
{
  for(int n = 0; n < nbells; ++n) fRow.push_back(n);
  /*
  for(int i = 0; i < 200; ++i){
    ApplyChange();
    for(int n = 0; n < nbells; ++n) std::cout << fRow[n]+1 << " ";
    std::cout << std::endl;
  }
  */
}

int Method::BellAt(int tick)
{
  // Open handstroke lead
  if(tick % (fRow.size()*2+1) == 0) return -1;

  // Count how many handstroke leads there have been and correct for them
  tick -= tick/(fRow.size()*2+1)+1;

  // Three whole pulls in rounds before go into changes. The first row of
  // "changes" is rounds the way this class thinks.
  const int kNumRoundsTicks = fRow.size()*5;

  if(tick < kNumRoundsTicks) return tick%fRow.size();

  tick -= kNumRoundsTicks;

  if(tick <= fTick) return fRow[fRowPos];

  ++fRowPos;
  ++fTick;
  if(fRowPos == fRow.size()){
    fRowPos = 0;
    if(fNotation != "-") ApplyChange();
  }

  std::cout << fRow[fRowPos]+1 << " ";
  if(fRowPos == fRow.size()-1) std::cout << std::endl;
  return fRow[fRowPos];
}

void Method::ApplyChange()
{
  if(fNotation[fNotationPos] == 'x'){
    for(int i = 0; i < fRow.size()-1; i += 2) std::swap(fRow[i], fRow[i+1]);
    ++fNotationPos;
  }
  else{ // not an x
    for(int i = 0; i < fRow.size()-1; i += 2){
      while(fNotationPos < fNotation.size() && CharToBell(fNotation[fNotationPos]) == i){
	++fNotationPos;
	++i;
      }
      // Don't swap last bell off the back
      if(i < fRow.size()-1) std::swap(fRow[i], fRow[i+1]);
    }
    // Eat trailing numbers that were superfluous
    while(fNotationPos < fNotation.size() && CharToBell(fNotation[fNotationPos]) >= 0){
      ++fNotationPos;
    }
    // Eat a dot if there is one
    if(fNotationPos < fNotation.size() && fNotation[fNotationPos] == '.') ++fNotationPos;
  }

  if(fNotationPos == fNotation.size()) fNotationPos = 0;
}

int Method::CharToBell(char chr) const
{
  if(chr == '0') return 9;
  if(chr == 'E') return 10;
  if(chr == 'T') return 11;
  if(chr >= '1' && chr <= '9') return chr-'1';
  return -1; // Dots and so on
}
