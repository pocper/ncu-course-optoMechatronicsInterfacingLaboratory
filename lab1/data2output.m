clear;
load data_sin.mat;
for i=1:200
    output_sin(i)=data0(i);
end
clear data0;
load data_cos.mat;
for i=1:200
    output_cos(i)=data0(i);
end

save('output_sin.mat','output_sin');
save('output_cos.mat','output_cos');