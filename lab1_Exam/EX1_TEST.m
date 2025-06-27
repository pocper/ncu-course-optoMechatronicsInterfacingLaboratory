clear; clc; close;
% fclose(instrfind);

[port] = remo_open(5);

[msg] = remo_get_msg(port);
disp(msg); %Enter A , B , C , D , x0

[data] = remo_snget_struct(port);
disp(data.field1); %0.5 0.45 0.35 0.25 0.15

remo_snput_struct(port, data);

[msg] = remo_get_msg(port);
disp(msg); %A=0.5 B=0.45 C=0.35 D=0.25 x0=0.15

[msg] = remo_get_msg(port);
disp(msg); %Enter u(k)

%load sin_1.mat
load sin_2.mat
remo_snput_matrix(port, sin_2);

[msg] = remo_get_msg(port);
disp(msg); %Output y(k)

%[output_sin_1] = remo_snget_matrix(port);
[output_sin_2] = remo_snget_matrix(port);

remo_close(port);
%save('output_sin_1.mat', 'output_sin_1');
save('output_sin_2.mat', 'output_sin_2');

disp('Finish Send!');
%%
load sin_1.mat
load sin_2.mat
sin = [sin_1, sin_2];
clear sin_1 sin_2;
load output_sin_1.mat
load output_sin_2.mat
output_sin = [output_sin_1, output_sin_2];
clear output_sin_1 output_sin_2;

t = linspace(-0.5, 0.5, 300);
hold on;
plot(t, sin);
plot(t, output_sin);
xlabel('時間[s]');
ylabel('軸度');
legend('輸入', '輸出')
saveas(gcf, 'EX1_TEST', 'jpeg');
hold off;
