#include <iostream>
#include <string>
using namespace std;

#ifndef NATIVE_SYSTEMC
#include "esc.h"
#endif

// Wall Clock Time Measurement
#include <sys/time.h>

#include "System.h"

// TIMEVAL STRUCT IS Defined ctime
// use start_time and end_time variables to capture
// start of simulation and end of simulation
struct timeval start_time, end_time;

System * sys = NULL;

#ifndef NATIVE_SYSTEMC
extern void esc_elaborate()
{
	sys = new System("sys");
}
extern void esc_cleanup()
{
	delete sys;
}
#endif

// int main(int argc, char *argv[])
int sc_main(int argc, char **argv) {

#ifndef NATIVE_SYSTEMC
	esc_initialize(argc, argv);
#endif
	
#ifndef NATIVE_SYSTEMC
	esc_elaborate();
#else
	sys = new System("sys");
#endif
	sc_start();
#ifndef NATIVE_SYSTEMC
	esc_cleanup();
#else
	delete sys;
#endif
	std::cout<< "Simulated time == " << sc_core::sc_time_stamp() << std::endl;

  return 0;
}
