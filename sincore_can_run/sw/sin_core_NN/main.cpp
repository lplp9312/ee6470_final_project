#include <string.h>
#include "stdio.h"

//#include "testset_06.h"
#include "testset_30.h"

// Filter ACC
static char* const FILTER_START_ADDR = reinterpret_cast<char* const>(0x75000000);
static char* const FILTER_READ_ADDR  = reinterpret_cast<char* const>(0x75000004);

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;

bool _is_using_dma = false;

void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){  
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(buffer);
    *DMA_DST_ADDR = (uint32_t)(ADDR);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Send
    memcpy(ADDR, buffer, sizeof(unsigned char)*len);
  }
}
void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len){
  if(_is_using_dma){
    // Using DMA 
    *DMA_SRC_ADDR = (uint32_t)(ADDR);
    *DMA_DST_ADDR = (uint32_t)(buffer);
    *DMA_LEN_ADDR = len;
    *DMA_OP_ADDR  = DMA_OP_MEMCPY;
  }else{
    // Directly Read
    memcpy(buffer, ADDR, sizeof(unsigned char)*len);
  }
}

int main() {
  unsigned int width = 1024;
  unsigned int testset_num = 30;
  word temp;
  unsigned char buffer[4] = {0};

  for (unsigned int i = 0; i < testset_num; i++) {
    //printf("\n===================\n\n");
    //printf("Type %d testset\n",golden[i]);
    for (unsigned int j = 0; j < width; j++) {
      temp.sint = testset[i * width + j]*1000000;
      buffer[0] = temp.uc[0];
      buffer[1] = temp.uc[1];
      buffer[2] = temp.uc[2];
      buffer[3] = temp.uc[3];
      write_data_to_ACC(FILTER_START_ADDR, buffer, 4);
    }
    read_data_from_ACC(FILTER_READ_ADDR, buffer, 4);
    temp.uc[0] = buffer[0];
    temp.uc[1] = buffer[1];
    temp.uc[2] = buffer[2];
    temp.uc[3] = buffer[3];
    //printf("Prediction : Type %d\n",temp.sint);
  }
}
