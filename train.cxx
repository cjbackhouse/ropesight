#include <cmath>
#include <cstdio>

#include <iostream>

int main()
{
  const double g = 12;

  const double dt = 0.01;

  std::cout << "Running simulations for training file, please wait..." << std::endl;

  FILE* f = fopen("train.txt", "w");

  for(int iang = 0; iang < 1000; ++iang){
    if(iang%10 == 0) (std::cout << "\r" << iang/10 << " / 100").flush();
    for(int ivel = 0; ivel < 1000; ++ivel){

      double ang = iang/1000.*M_PI;
      double vel = (ivel/1000.-.5)*20;

      double tott = 0;

      bool pulling = true;

      while(true){
	tott += dt;

	vel -= g*dt*sin(ang);

	// Velocity that will cause the bell to come most of the way to the
	// balance
	const double target = sqrt(2*g*(cos(ang)-cos(3.05)));

	// Higher values make the bell respond more quickly to your pull, which
	// makes it easier. But eventually it'll be unrealistic.
	const double kPullForce = 3;

	if(pulling){
	  if(vel > -target){
	    vel -= kPullForce*dt;
	  }
	  if(vel < -target){
	    pulling = false;
	    vel = -target;
	  }
	}
	
	ang += dt*vel;
      
	if(ang < 0){
	  fprintf(f, "%d %d %lf %lf %lf\n", iang, ivel, ang, vel, tott);
	  break;
	}

	if(ang > M_PI){
	  // Bell goes over the balance. Don't write anything, this situation
	  // should never happen.
	  break;
	}
      } // end while
    }
  }

  fclose(f);

  std::cout << "\rDone: wrote train.txt" << std::endl;

  return 0;
}
