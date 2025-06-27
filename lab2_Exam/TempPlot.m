clear; clc; close;
% fclose(instrfind);

[port] = remo_open(7);

[msg] = remo_get_msg(port);
disp(msg);

t = 0;
temp=zeros(1,2);

while (1)
    %     [msg] = remo_get_msg(port);
    %     disp(msg);
    hold on;
    [temperature] = remo_snget_matrix(port);
%     disp(['t=', num2str(t), ' Temp=', num2str(temperature)]);
    plot(t, single(temperature), 'k.');
    xlabel('時間[s]');
    ylabel('溫度[°C]');
    t = t + 1;
end

remo_close(port);
disp('Finish Send!');
