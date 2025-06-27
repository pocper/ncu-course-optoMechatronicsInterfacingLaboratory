clear; clc; close;
% fclose(instrfind);

[port] = remo_open(5);

[msg] = remo_get_msg(port);
disp(msg); %Enter A , B , C , D , x0

[data] = remo_snget_struct(port);
disp(data.field1); %1.000 0.7500 0.5000 0.25000 0

remo_snput_struct(port, data);

[msg] = remo_get_msg(port);
disp(msg); %A=1.000 B=0.7500 C=0.5000 D=0.25000 x0=0

[msg] = remo_get_msg(port);
disp(msg); %Enter u(k)

load sin.mat
remo_snput_matrix(port, sin);

[msg] = remo_get_msg(port);
disp(msg); %Output y(k)

[output_sin] = remo_snget_matrix(port);

remo_close(port);
disp('Finish Send!');
%%
%fclose(instrfind);

[port] = remo_open(5);

[msg] = remo_get_msg(port);
disp(msg); %Enter A , B , C , D , x0

[data] = remo_snget_struct(port);
disp(data.field1); %1 -1 1 -1 1

remo_snput_struct(port, data);

[msg] = remo_get_msg(port);
disp(msg); %A=1 B=-1 C=1 D=-1 x0=1

[msg] = remo_get_msg(port);
disp(msg); %Enter u(k)

load cos.mat
remo_snput_matrix(port, cos);

[msg] = remo_get_msg(port);
disp(msg); %Output y(k)

[output_cos] = remo_snget_matrix(port);

remo_close(port);
disp('Finish Send!');
%%
t = linspace(-0.5, 0.5, 200);
hold on;
plot(t, sin);
plot(t, output_sin);
xlabel('時間[s]');
ylabel('軸度');
legend('輸入', '輸出')
saveas(gcf, 'EX2A', 'jpeg');
hold off;
close;
hold on;
plot(t, cos);
plot(t, output_cos);
xlabel('時間[s]');
ylabel('軸度');
legend('輸入', '輸出')
saveas(gcf, 'EX2B', 'jpeg');
hold off;
