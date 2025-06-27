clear;clc;close;

port = remo_open(6);

t = linspace(0, 2 * pi, 180);
x = uint16((sin(t)+1) * 255/2);

volt_ref = 4.65;

remo_snput_matrix(port, x);

while 1
    [data1] = remo_snget_matrix(port);
    [data2] = remo_snget_matrix(port);
    data1 = double(data1)/1024*volt_ref;
    data2 = double(data2)/1024*volt_ref;
    
    plot(t,data1,t,data2);
    xlim([0 2*pi]);
    ylim([0 volt_ref]);
    xlabel('Time[t]');
    ylabel('Volt[V]');
    title('PWM Three-phase Sine Wave');
    legend('sin(t)','sin(t+\pi/3)');
end

remo_close(port);
