clear;clc;close;

t = linspace(0, pi, 200);
x = square(t, 25);

for i = 1:200

    if (x(i) < 0)
        x(i) = 0;
    end

end

[port] = remo_open(8);
remo_snput_matrix(port, uint16(x));

while 1
    [data] = remo_snget_matrix(port);
    
    subplot(1, 2, 1);
    plot(t, 2 * x);
    title('Input PWM');
    xlabel('Time[t]');
    ylabel('Voltage[V]');
    ylim([0, 3]);
    
    subplot(1, 2, 2);
    data = data * 2.56/1024;
    plot(t, data);
    title('ADC convert Output PWM');
    xlabel('Time[t]');
    ylabel('Voltage[V]');
    ylim([0, 3]);
end

remo_close(port);
