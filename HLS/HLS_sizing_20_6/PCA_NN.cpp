#ifndef NATIVE_SYSTEMC
#include "stratus_hls.h"
#endif

#include "PCA_NN.h"

#include "coeff.h"
#include "mean.h"
#include "para.h"

PCA_NN::PCA_NN(sc_module_name n) : sc_module(n)
{

	SC_THREAD(do_filter);
	sensitive << i_clk.pos();
	dont_initialize();
	reset_signal_is(i_rst, false);

#ifndef NATIVE_SYSTEMC
	i_data.clk_rst(i_clk, i_rst);
	o_result.clk_rst(i_clk, i_rst);
#endif
}

PCA_NN::~PCA_NN() {}

void PCA_NN::do_filter()
{
	{
#ifndef NATIVE_SYSTEMC
		i_data.reset();
		o_result.reset();
#endif
		wait();
	}

	sc_fixed<20, 6> input_data;
	sc_fixed<20, 6> pca_result[2];
	sc_fixed<20, 6> hidden_o[4];
	sc_fixed<20, 6> output_o[3];
	sc_uint<4> prediction;

	while (true)
	{

		pca_result[0] = 0;
		pca_result[1] = 0;

		for (sc_uint<12> i = 0; i < 1024; i++)
		{

#ifndef NATIVE_SYSTEMC
			input_data = i_data.get();
#else
			input_data = i_data.read();
#endif

			input_data -= mean[i];
			pca_result[0] += input_data * coeff[i];
			wait();
			pca_result[1] += input_data * coeff[1024 + i];
			wait();
		}

		//cout << "[P1, P2] = "
		//	 << "[" << pca_result[0] << ", " << pca_result[1] << "]" << endl;

		for (sc_uint<4> i = 0; i < 4; i++)
		{
			hidden_o[i] = pca_result[0] * hidden_w[2 * i] + pca_result[1] * hidden_w[2 * i + 1] + hidden_b[i];
			wait();
		}

		//cout << "Hidden layer output = "
		//	 << "[" << hidden_o[0] << ", " << hidden_o[1] << ", " << hidden_o[2] << ", " << hidden_o[3] << "]" << endl;

		for (sc_uint<4> i = 0; i < 4; i++)
		{

			//hidden_o[i] = 1 / (1 + exp(-1 * hidden_o[i]));
			if (hidden_o[i] >= 4)
			{
				hidden_o[i] = 1;
			}
			else if (hidden_o[i] <= -4)
			{
				hidden_o[i] = 0;
			}
			else
			{
				hidden_o[i] = 1 / 2 + hidden_o[i] / 8;
			}
			wait();
		}

		for (sc_uint<4> i = 0; i < 3; i++)
		{
			output_o[i] = 0;
			for (sc_uint<4> j = 0; j < 4; j++)
			{
				output_o[i] += hidden_o[j] * output_w[4 * i + j];
				wait();
			}
			output_o[i] += output_b[i];
		}

		//cout << "Output layer output = "
		//	 << "[" << output_o[0] << ", " << output_o[1] << ", " << output_o[2] << "]" << endl;

		if ((output_o[0] >= output_o[1]) && (output_o[0] >= output_o[2]))
		{
			prediction = 1;
		}
		else if ((output_o[1] >= output_o[0]) && (output_o[1] >= output_o[2]))
		{
			prediction = 2;
		}
		else
		{
			prediction = 3;
		}

		wait();

#ifndef NATIVE_SYSTEMC
		o_result.put(prediction);
#else
		o_result.write(prediction);
#endif
	}
}
