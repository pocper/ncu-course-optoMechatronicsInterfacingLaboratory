clear;clc;close;
[port] = remo_open(6);

% 0 2V
% 0 4096
outputVoltage = linspace(0, 1, 21);
data = uint16(outputVoltage * (2^12/2));
remo_snput_matrix(port, data);

while 1
    [data] = remo_get_msg(port);
    disp(data);
end

remo_close(port);
