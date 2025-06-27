clear;clc;close;
[port] = remo_open(8);

var = input('enter arrSize = ');
remo_snput_matrix(port,uint8(var));

t = linspace(0,2*pi,uint8(var));

y = sin(t);
y = uint16((y + 1) / 2 * (2^12 -1));

while 1
    remo_snput_matrix(port,y);
end

remo_close(port);