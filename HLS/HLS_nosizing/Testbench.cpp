#include <cstdio>
#include <cstdlib>
#include <string.h>
#include "stdio.h"

#include "Testbench.h"
//#include "testset_06.h"
#include "testset_30.h"

Testbench::Testbench(sc_module_name n) : sc_module(n)
{
  SC_THREAD(testbench);
  sensitive << i_clk.pos();
  dont_initialize();
}

Testbench::~Testbench()
{
  cout << "Total run time = " << total_run_time << endl;
}

void Testbench::testbench()
{

#ifndef NATIVE_SYSTEMC
  o_data.reset();
  i_result.reset();
#endif

  o_rst.write(false);
  wait(5);
  o_rst.write(true);
  wait(1);
  total_start_time = sc_time_stamp();

  unsigned int width = 1024;
  unsigned int testset_num = 30;
  unsigned int temp;

  for (unsigned int i = 0; i < testset_num; i++)
  {
    //printf("\n===================\n\n");
    //printf("Type %d testset\n", golden[i]);
    for (unsigned int j = 0; j < width; j++)
    {

#ifndef NATIVE_SYSTEMC
      o_data.put(testset[i * width + j]);
#else
      o_data.write(testset[i * width + j]);
#endif
      wait();
    }

#ifndef NATIVE_SYSTEMC
    temp = i_result.get();
#else
    temp = i_result.read();
#endif
    wait();
    //printf("Prediction : Type %d\n", temp);
    if (temp != golden[i])
    {
      printf("Error %d\n", i);
    }
  }

  total_run_time = sc_time_stamp() - total_start_time;
  sc_stop();
}
