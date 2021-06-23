#include <string.h>
#include "stdio.h"

//#include "testset_06.h"
#include "testset_30.h"

//Total number of cores
//static const int PROCESSORS = 2;
#define PROCESSORS 2

//Filter ACC
static char* const FILTER_START_ADDR[PROCESSORS] = {reinterpret_cast<char* const>(0x73000000),reinterpret_cast<char* const>(0x74000000)};
static char* const FILTER_READ_ADDR[PROCESSORS] = {reinterpret_cast<char* const>(0x73000004),reinterpret_cast<char* const>(0x74000004)};

// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;
static const uint32_t DMA_OP_NOP = 0;

//TODO fixed DMA access
bool _is_using_dma = false;
//the barrier synchronization objects
uint32_t barrier_counter=0; 
uint32_t barrier_lock; 
uint32_t barrier_sem; 
//the mutex object to control global summation
uint32_t lock;  
//print synchronication semaphore (print in core order)
uint32_t print_sem[PROCESSORS]; 


int sem_init (uint32_t *__sem, uint32_t count) __THROW{
  *__sem=count;
  return 0;
}
int sem_wait (uint32_t *__sem) __THROW{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     beqz %[value],L%=                   # if zero, try again\n\t\
     addi %[value],%[value],-1           # value --\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int sem_post (uint32_t *__sem) __THROW{
  uint32_t value, success; //RV32A
  __asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     addi %[value],%[value], 1           # value ++\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  return 0;
}

int barrier(uint32_t *__sem, uint32_t *__lock, uint32_t *counter, uint32_t thread_count) {
	sem_wait(__lock);
	if (*counter == thread_count - 1) { //all finished
		*counter = 0;
		sem_post(__lock);
		for (int j = 0; j < thread_count - 1; ++j) sem_post(__sem);
	} else {
		(*counter)++;
		sem_post(__lock);
		sem_wait(__sem);
	}
	return 0;
}

void write_data_to_ACC(char* ADDR, unsigned char* buffer, int len, int hart_id){
    if(_is_using_dma){  
        // Using DMA 
	    sem_wait(&lock);
        *(DMA_SRC_ADDR) = (uint32_t)(buffer);
        *(DMA_DST_ADDR) = (uint32_t)(ADDR);
        *(DMA_LEN_ADDR) = len;
        *(DMA_OP_ADDR)  = DMA_OP_MEMCPY;
	    sem_post(&lock);

    }else{
        // Directly Send
        memcpy(ADDR, buffer, sizeof(unsigned char)*len);
    }
}
void read_data_from_ACC(char* ADDR, unsigned char* buffer, int len, int hart_id){
    if(_is_using_dma){
        // Using DMA 
	    sem_wait(&lock);
        *(DMA_SRC_ADDR) = (uint32_t)(ADDR);
        *(DMA_DST_ADDR) = (uint32_t)(buffer);
        *(DMA_LEN_ADDR) = len;
        *(DMA_OP_ADDR)  = DMA_OP_MEMCPY;
	    sem_post(&lock);
    }else{
        // Directly Read
        memcpy(buffer, ADDR, sizeof(unsigned char)*len);
    }
}

int main(unsigned hart_id) {

	if (hart_id == 0) {
		// create a barrier object with a count of PROCESSORS
		sem_init(&barrier_lock, 1);
		sem_init(&barrier_sem, 0); //lock all cores initially
		for(int i=0; i< 2; ++i){
			sem_init(&print_sem[i], 0); //lock printing initially
		}
		// Create mutex lock
		sem_init(&lock, 1);
	}

  unsigned int print_id = 0;
  unsigned int width = 1024;
  unsigned int testset_num = 30;
  word temp;
  unsigned char buffer[4] = {0};
  unsigned int testset_num_start = (hart_id == 0) ? 0 : testset_num/2;
	unsigned int testset_num_end = (hart_id == 0) ? testset_num/2 : testset_num;

  //if (hart_id == print_id) 
  //  printf("core %d \n",print_id);
  for (unsigned int i = testset_num_start; i < testset_num_end; i++) {
  /*  if (hart_id == print_id) {
      printf("\n============================\n");
      printf("Type %d testset\n",golden[i]);
    }*/
    for (unsigned int j = 0; j < width; j++) {
      temp.sint = testset[i * width + j]*1000000;
      buffer[0] = temp.uc[0];
      buffer[1] = temp.uc[1];
      buffer[2] = temp.uc[2];
      buffer[3] = temp.uc[3];
      write_data_to_ACC(FILTER_START_ADDR[hart_id], buffer, 4, hart_id);
    }
    read_data_from_ACC(FILTER_READ_ADDR[hart_id], buffer, 4, hart_id);
    temp.uc[0] = buffer[0];
    temp.uc[1] = buffer[1];
    temp.uc[2] = buffer[2];
    temp.uc[3] = buffer[3];
  //  if (hart_id == print_id) 
  //    printf("Prediction : Type %d\n",temp.sint);
  }
}
