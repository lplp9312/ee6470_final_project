clear all
clc

trainset_num = 490;
testset_num = 166;

%% Load training data 
for i = 0:trainset_num-1
    
    imagefile = sprintf('Data/Data_train/Carambula/Carambula_train_%d.png',i);
    image = im2double(imread(imagefile));
    trainset(:,i+1) = reshape(image,[size(image,1)*size(image,2),1]);
    train_id(:,i+1) = [1;0;0];
    
    imagefile = sprintf('Data/Data_train/Lychee/Lychee_train_%d.png',i);
    image = im2double(imread(imagefile));
    trainset(:,i+1+trainset_num) = reshape(image,[size(image,1)*size(image,2),1]);
    train_id(:,i+1+trainset_num) = [0;1;0];
    
    imagefile = sprintf('Data/Data_train/Pear/Pear_train_%d.png',i);
    image = im2double(imread(imagefile));
    trainset(:,i+1+2*trainset_num) = reshape(image,[size(image,1)*size(image,2),1]);
    train_id(:,i+1+2*trainset_num) = [0;0;1];
    
end

%% Load testing data 
for i = 0:testset_num-1
    
    imagefile = sprintf('Data/Data_test/Carambula/Carambula_test_%d.png',i);
    image = im2double(imread(imagefile));
    testset(:,i+1) = reshape(image,[size(image,1)*size(image,2),1]);
    test_id(:,i+1) = [1;0;0];
    
    imagefile = sprintf('Data/Data_test/Lychee/Lychee_test_%d.png',i);
    image = im2double(imread(imagefile));
    testset(:,i+1+testset_num) = reshape(image,[size(image,1)*size(image,2),1]);
    test_id(:,i+1+testset_num) = [0;1;0];
    
    imagefile = sprintf('Data/Data_test/Pear/Pear_test_%d.png',i);
    image = im2double(imread(imagefile));
    testset(:,i+1+2*testset_num) = reshape(image,[size(image,1)*size(image,2),1]);
    test_id(:,i+1+2*testset_num) = [0;0;1];
    
end

%% Principal component analysis
[coeff,score,latent] = pca([trainset testset]','NumComponents',2);
trainset2D = score(1:3*trainset_num,:);
testset2D = score(3*trainset_num+1:3*(trainset_num + testset_num),:);
test0 = [trainset testset]';
test1 = mean(test0);
test2 = test0 - test1;
test3 = test2*coeff;

%% Build a two-layer neural network
input_num = 2;
hidden_num = 4;
output_num = 3;

% Bias
hidden_layer_bias = -1+2*rand(hidden_num,1);
output_layer_bias = -1+2*rand(output_num,1);

% Weight
hidden_layer_weight = -1+2.*rand(hidden_num,input_num);
output_layer_weight = -1+2.*rand(output_num,hidden_num);

% Learning coefficient
learning_coeff = 0.08;

% Learning iteration
learning_iteration = 1000;

% Training
for i = 1:learning_iteration
    p = randperm(3*trainset_num);
    trainset_rand = trainset2D(p,:);
    train_id_rand = train_id(:,p);
    
    for j = 1:3*trainset_num
        input = trainset_rand(j,:)';
        
        % Hidden layer
        hidden_layer_output = hidden_layer_weight*input+hidden_layer_bias;
        hidden_layer_output = 1./(1+exp(-1*hidden_layer_output));
        
        % Output layer
        output_layer_output = output_layer_weight*hidden_layer_output+output_layer_bias;
        output_layer_output = 1./(1+exp(-1*output_layer_output));
        
        % Backpropagation
        delta2 = output_layer_output.*(1-output_layer_output).*(train_id_rand(:,j)-output_layer_output);
        delta1 = output_layer_weight'*delta2.*hidden_layer_output.*(1-hidden_layer_output);
        
        hidden_layer_weight = hidden_layer_weight+delta1*learning_coeff*input';
        output_layer_weight = output_layer_weight+delta2*learning_coeff*hidden_layer_output';
        
        hidden_layer_bias = hidden_layer_bias+learning_coeff*delta1;
        output_layer_bias = output_layer_bias+learning_coeff*delta2;
    end
end

%% Accuracy of the testing data
acc = 0;
for i = 1:3*testset_num
    input = testset2D(i,:)';
    
    hidden_layer_output = hidden_layer_weight*input+hidden_layer_bias;
    hidden_layer_output = 1./(1+exp(-1*hidden_layer_output));
    
    output_layer_output = output_layer_weight*hidden_layer_output+output_layer_bias;
    output_layer_output = 1./(1+exp(-1*output_layer_output));
    
    prediction = find(output_layer_output==max(output_layer_output));
    
    if(prediction == find(test_id(:,i)==max(test_id(:,i))))
        acc = acc+1/(3*testset_num);
    end
end
%{
%% Plot
t=-8:0.1:8;

Carambula_counter=1;
Lychee_counter=1;
Pear_counter=1;

for i = 1:length (t)
    for j = 1:length (t)
        input=[t(i);t(j)];
        
        hidden_layer_output = hidden_layer_weight*input+hidden_layer_bias;
        hidden_layer_output = 1./(1+exp(-1*hidden_layer_output));
        
        output_layer_output = output_layer_weight*hidden_layer_output+output_layer_bias;
        output_layer_output = 1./(1+exp(-1*output_layer_output));
        
        prediction = find(output_layer_output==max(output_layer_output));
        
        if (prediction == 1)
            Carambula_decision_boundary(:,Carambula_counter) = [t(i),t(j)];
            Carambula_counter = Carambula_counter+1;
        end
        if (prediction == 2)
            Lychee_decision_boundary(:,Lychee_counter) = [t(i),t(j)];
            Lychee_counter = Lychee_counter+1;
        end
        if (prediction == 3)
            Pear_decision_boundary(:,Pear_counter)=[t(i),t(j)];
            Pear_counter = Pear_counter+1;
        end
        
    end
end

% Decision region and training performance 
figure
plot(Carambula_decision_boundary(1,:),Carambula_decision_boundary(2,:),'r.','MarkerSize',10)
hold on
plot(trainset2D(1:trainset_num,1),trainset2D(1:trainset_num,2),'x','color',[0.5 1 1])
legend('Carambula boundary','Carambula')
xlabel('P1')
ylabel('P2')
title('trainset data and decision region')

figure
plot(Lychee_decision_boundary(1,:),Lychee_decision_boundary(2,:),'g.','MarkerSize',10)
hold on
plot(trainset2D(trainset_num+1:2*trainset_num,1),trainset2D(trainset_num+1:2*trainset_num,2),'o','color',[1 0.5 1])
legend('Lychee boundary','Lychee')
xlabel('P1')
ylabel('P2')
title('trainset data and decision region')

figure
plot(Pear_decision_boundary(1,:),Pear_decision_boundary(2,:),'b.','MarkerSize',10)
hold on
plot(trainset2D(2*trainset_num+1:3*trainset_num,1),trainset2D(2*trainset_num+1:3*trainset_num,2),'*','color',[1 1 0.5])
legend('Pear boundary','Pear')
xlabel('P1')
ylabel('P2')
title(' trainset data and decision region')

% Decision region and testing performance
figure
plot(Carambula_decision_boundary(1,:),Carambula_decision_boundary(2,:),'r.','MarkerSize',10)
hold on
plot(testset2D(1:testset_num,1),testset2D(1:testset_num,2),'x','color',[0.5 1 1])
legend('Carambula boundary','Carambula')
xlabel('P1')
ylabel('P2')
title('testset data and decision region')

figure
plot(Lychee_decision_boundary(1,:),Lychee_decision_boundary(2,:),'g.','MarkerSize',10)
hold on
plot(testset2D(testset_num+1:2*testset_num,1),testset2D(testset_num+1:2*testset_num,2),'o','color',[1 0.5 1])
legend('Lychee boundary','Lychee')
xlabel('P1')
ylabel('P2')
title('testset data and decision region')

figure
plot(Pear_decision_boundary(1,:),Pear_decision_boundary(2,:),'b.','MarkerSize',10)
hold on
plot(testset2D(2*testset_num+1:3*testset_num,1),testset2D(2*testset_num+1:3*testset_num,2),'*','color',[1 1 0.5])
legend('Pear boundary','Pear')
xlabel('P1')
ylabel('P2')
title('testset data and decision region')
%}
