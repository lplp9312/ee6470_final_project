#ifndef NN_H_
#define NN_H_
#include <systemc>
#include <cmath>
#include <iomanip>
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "coeff.h"
#include "mean.h"
#include "para.h"

struct NN : public sc_module
{
  tlm_utils::simple_target_socket<NN> tsock;

  sc_fifo<int> input;
  sc_fifo<int> output;

  SC_HAS_PROCESS(NN);

  NN(sc_module_name n) : sc_module(n),
                         tsock("t_skt")
  {
    tsock.register_b_transport(this, &NN::blocking_transport);
    SC_THREAD(do_filter);
  }

  ~NN() {}

  void do_filter()
  {
    {
      wait(CLOCK_PERIOD, SC_NS);
    }

    double input_data;
    double pca_result[2];
    double hidden_o[4];
    double output_o[3];
    int prediction;

    while (true)
    {

      pca_result[0] = 0;
      pca_result[1] = 0;

      for (unsigned int i = 0; i < 1024; i++) {
        input_data = input.read();
        input_data = input_data / 1000000;
        input_data -= mean[i];
        pca_result[0] += input_data * coeff[i];
        wait(CLOCK_PERIOD, SC_NS);
        pca_result[1] += input_data * coeff[1024 + i];
        wait(CLOCK_PERIOD, SC_NS);
      }

      //std::cout << "[P1, P2] = " << "[" << pca_result[0] << ", " << pca_result[1] << "]" << std::endl;

      for (unsigned int i = 0; i < 4; i++) {
        hidden_o[i] = pca_result[0] * hidden_w[2 * i] + pca_result[1] * hidden_w[2 * i + 1] + hidden_b[i];
        wait(CLOCK_PERIOD, SC_NS);
      }

      //std::cout << "Hidden layer output = "
      //          << "[" << hidden_o[0] << ", " << hidden_o[1] << ", " << hidden_o[2] << ", " << hidden_o[3] << "]" << std::endl;

      for (unsigned int i = 0; i < 4; i++) {
        hidden_o[i] = 1 / (1 + exp(-1 * hidden_o[i]));
        wait(CLOCK_PERIOD, SC_NS);
      }

      for (unsigned int i = 0; i < 3; i++) {
        output_o[i] = 0;
        for (unsigned int j = 0; j < 4; j++) {
          output_o[i] += hidden_o[j] * output_w[4 * i + j];
          wait(CLOCK_PERIOD, SC_NS);
        }
        output_o[i] += output_b[i];
      }

      //std::cout << "Output layer output = " << "[" << output_o[0] << ", " << output_o[1] << ", " << output_o[2] << "]" << std::endl;      

      for (unsigned int i = 0; i < 3; i++) {
        output_o[i] = 1 / (1 + exp(-1 * output_o[i]));
        wait(CLOCK_PERIOD, SC_NS);
      }

      if ((output_o[0] >= output_o[1]) && (output_o[0] >= output_o[2])) {
        prediction = 1;
      }
      else if ((output_o[1] >= output_o[0]) && (output_o[1] >= output_o[2])) {
        prediction = 2;
      }
      else {
        prediction = 3;
      }
      wait(CLOCK_PERIOD, SC_NS);
      output.write(prediction);
    }
  }

  void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay)
  {
    wait(delay);
    tlm::tlm_command cmd = payload.get_command();
    sc_dt::uint64 addr = payload.get_address();
    unsigned char *data_ptr = payload.get_data_ptr();

    word buffer;

    switch (cmd)
    {
    case tlm::TLM_READ_COMMAND:
      switch (addr)
      {
      case FILTER_RESULT_ADDR:
        buffer.sint = output.read();
        break;
      default:
        buffer.uc[0] = 0;
        buffer.uc[1] = 0;
        buffer.uc[2] = 0;
        buffer.uc[3] = 0;
        std::cerr << "READ Error! NN::blocking_transport: address 0x"
                  << std::setfill('0') << std::setw(8) << std::hex << addr
                  << std::dec << " is not valid" << std::endl;
      }
      data_ptr[0] = buffer.uc[0];
      data_ptr[1] = buffer.uc[1];
      data_ptr[2] = buffer.uc[2];
      data_ptr[3] = buffer.uc[3];
      break;
    case tlm::TLM_WRITE_COMMAND:
      switch (addr)
      {
      case FILTER_R_ADDR:
        buffer.uc[0] = data_ptr[0];
        buffer.uc[1] = data_ptr[1];
        buffer.uc[2] = data_ptr[2];
        buffer.uc[3] = data_ptr[3];
        input.write(buffer.sint);
        break;
      default:
        std::cerr << "WRITE Error! NN::blocking_transport: address 0x"
                  << std::setfill('0') << std::setw(8) << std::hex << addr
                  << std::dec << " is not valid" << std::endl;
      }
      break;
    case tlm::TLM_IGNORE_COMMAND:
      payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
      return;
    default:
      payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
      return;
    }
    payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
  }
};
#endif
