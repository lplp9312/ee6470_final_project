#ifndef PCA_NN_H_
#define PCA_NN_H_
#include <systemc>
#include "cynw_fixed.h"

#ifndef NATIVE_SYSTEMC
#include <cynw_p2p.h>
#endif

using namespace sc_core;
using namespace sc_dt;
using namespace std;

class PCA_NN: public sc_module
{
public:
	sc_in_clk i_clk;
	sc_in < bool > i_rst;
#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_fixed<20, 6> >::in i_data;
	cynw_p2p< sc_uint<4> >::out o_result;
#else
	sc_fifo_in< sc_fixed<20, 6> > i_data;
	sc_fifo_out< sc_uint<4> > o_result;
#endif
	SC_HAS_PROCESS(PCA_NN);
	PCA_NN(sc_module_name n);
	~PCA_NN();

private:
	void do_filter();
};
#endif
