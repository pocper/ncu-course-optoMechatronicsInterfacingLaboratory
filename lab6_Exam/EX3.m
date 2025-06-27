clear;clc;close;

port = remo_open(6);

t = linspace(0, 2 * pi, 180);
x = uint16((sin(t)+1) * 255/2);

remo_snput_matrix(port, x);

while 1
    [data] = remo_get_msg(port);
    disp(data);
end

remo_close(port);
