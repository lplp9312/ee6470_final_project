# Final Project Report 蕭詠倫 109061634

## Principal Component Analysis and Two-layer Neural Network

### Task : classify three kinds of fruits
#### 1.Carambula
#### 2.Lychee
#### 3.Pear
#### Each image 32 x 32 (1024) pixels
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/task.jpg)

### Architecture
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/arc.jpg)

### Parameters generate from Matlab
#### Principal Component Analysis (mean.h & coeff.h)
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/PCA_para.jpg)
#### Two-layer Neural Network (para.h)
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/NN_para.jpg)

### Activation Function
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/activation.jpg)

### RISCV-VP Platform single-core
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/sin_core_sim.jpg)

### RISCV-VP Platform multi-core
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/mul_core0_sim.jpg)
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/mul_core1_sim.jpg)

### Single-core vs Multi-core
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/sim_verse.jpg)

### HLS 3 Versions Setting
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/version.jpg)

### HLS Total Run Time & Area Result
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/HLS_result.jpg)

### RTL Summary page
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/HLS_01.jpg)
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/HLS_02.jpg)
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/HLS_03.jpg)
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/HLS_04.jpg)
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/HLS_05.jpg)
![image](https://github.com/lplp9312/ee6470_final_project/blob/master/picture/HLS_06.jpg)

### RISCV-VP Platform single-core & multi-core Source Code Presentation

#### Testbench code (single-core)
        int main() {
            unsigned int width = 1024;
            unsigned int testset_num = 30;
            word temp;
            unsigned char buffer[4] = {0};

            for (unsigned int i = 0; i < testset_num; i++) {
                for (unsigned int j = 0; j < width; j++) {
                    temp.sint = testset[i * width + j]*1000000;
                    buffer[0] = temp.uc[0];
                    buffer[1] = temp.uc[1];
                    buffer[2] = temp.uc[2];
                    buffer[3] = temp.uc[3];
                    write_data_to_ACC(FILTER_START_ADDR, buffer, 4);
                }
                read_data_from_ACC(FILTER_READ_ADDR, buffer, 4);
            }
        }
#### Testbench code (multi-core)
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

            for (unsigned int i = testset_num_start; i < testset_num_end; i++) {
                for (unsigned int j = 0; j < width; j++) {
                    temp.sint = testset[i * width + j]*1000000;
                    buffer[0] = temp.uc[0];
                    buffer[1] = temp.uc[1];
                    buffer[2] = temp.uc[2];
                    buffer[3] = temp.uc[3];
                    write_data_to_ACC(FILTER_START_ADDR[hart_id], buffer, 4, hart_id);
                }
                read_data_from_ACC(FILTER_READ_ADDR[hart_id], buffer, 4, hart_id);
            }
        }

#### Principal Component Analysis and Two-layer Neural Network sc_module
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

                //PCA
                for (unsigned int i = 0; i < 1024; i++) {
                    input_data = input.read();
                    input_data = input_data / 1000000;
                    input_data -= mean[i];
                    pca_result[0] += input_data * coeff[i];
                    wait(CLOCK_PERIOD, SC_NS);
                    pca_result[1] += input_data * coeff[1024 + i];
                    wait(CLOCK_PERIOD, SC_NS);
                }

                //Hidden layer output
                for (unsigned int i = 0; i < 4; i++) {
                    hidden_o[i] = pca_result[0] * hidden_w[2 * i] + pca_result[1] * hidden_w[2 * i + 1] + hidden_b[i];
                    wait(CLOCK_PERIOD, SC_NS);
                }

                //Hidden layer activation function
                for (unsigned int i = 0; i < 4; i++) {
                    hidden_o[i] = 1 / (1 + exp(-1 * hidden_o[i]));
                    wait(CLOCK_PERIOD, SC_NS);
                }

                //Output layer output
                for (unsigned int i = 0; i < 3; i++) {
                    output_o[i] = 0;
                    for (unsigned int j = 0; j < 4; j++) {
                        output_o[i] += hidden_o[j] * output_w[4 * i + j];
                        wait(CLOCK_PERIOD, SC_NS);
                    }
                    output_o[i] += output_b[i];
                }
     
                //Output layer activation function
                for (unsigned int i = 0; i < 3; i++) {
                    output_o[i] = 1 / (1 + exp(-1 * output_o[i]));
                    wait(CLOCK_PERIOD, SC_NS);
                }

                //Prediction
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

### Discussion

在本次的期末專題為Principal Component Analysis和Two-layer Neural Network，Principal Component Analysis把圖片進行降維再進到Two-layer Neural Network進行水果種類的預測。在Matlab的部分，Training set有1500張圖片，Testing set有500張圖片，精準度大概為85%。在RISCV-VP Platform和HLS，使用了不同的Activation Function，是因為在HLS使用fixed point取代floating point，內建的指數函數在fixed point和floating point之間的轉換在HLS無法被合成。HLS也因為使用該Activation Function，精準度也有下降。在RISCV-VP Platform，30張圖片都有正確預測出水果種類，而在HLS，30張圖片有1張預測錯誤。



