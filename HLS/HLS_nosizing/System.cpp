#include "System.h"
System::System( sc_module_name n): sc_module( n ), 
	tb("tb"), pca_nn("pca_nn"), clk("clk", CLOCK_PERIOD, SC_NS), rst("rst")
{
	tb.i_clk(clk);
	tb.o_rst(rst);
	pca_nn.i_clk(clk);
	pca_nn.i_rst(rst);

	tb.o_data(data);

	pca_nn.i_data(data);
	pca_nn.o_result(result);

	tb.i_result(result);

}

System::~System() {}
