#ifndef TESTBENCH_H_
#define TESTBENCH_H_

#include <systemc>
#include "cynw_fixed.h"

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

using namespace std;
using namespace sc_core;
using namespace sc_dt;

class Testbench : public sc_module
{
public:
  sc_in_clk i_clk;
  sc_out<bool> o_rst;
#ifndef NATIVE_SYSTEMC
  cynw_p2p< sc_fixed<32, 8> >::base_out o_data;
  cynw_p2p< sc_uint<32> >::base_in i_result;
#else
  sc_fifo_out< sc_fixed<32, 8> > o_data;
  sc_fifo_in< sc_uint<32> > i_result;
#endif

  SC_HAS_PROCESS(Testbench);
  Testbench(sc_module_name n);
  ~Testbench();

private:
  sc_time total_start_time;
  sc_time total_run_time;

  void testbench();
};
#endif
