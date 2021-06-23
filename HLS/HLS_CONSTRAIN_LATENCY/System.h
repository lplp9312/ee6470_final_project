
#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <systemc>

#include "Testbench.h"
#include "cynw_fixed.h"

#ifndef NATIVE_SYSTEMC
#include "PCA_NN_wrap.h"
#else
#include "PCA_NN.h"
#endif

using namespace sc_dt;
using namespace sc_core;

class System : public sc_module
{
public:
	SC_HAS_PROCESS(System);
	System(sc_module_name n);
	~System();

private:
	Testbench tb;

#ifndef NATIVE_SYSTEMC
	PCA_NN_wrapper pca_nn;
#else
	PCA_NN pca_nn;
#endif
	sc_clock clk;
	sc_signal<bool> rst;

#ifndef NATIVE_SYSTEMC
	cynw_p2p< sc_fixed<20, 6> > data;
	cynw_p2p< sc_uint<4> > result;
#else
	sc_fifo< sc_fixed<20, 6> > data;
	sc_fifo< sc_uint<4> > result;
#endif

};
#endif
