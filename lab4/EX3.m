clear;clc;close;

[port] = remo_open(8);
title('freq. 10Hz | V_{pp} 1V | offset 0.5V | sine wave ADC convert');
xlabel('set');
ylabel('Volt[V]');
ylim([0 ,0.6]);

hold on;
[data_single] = remo_snget_matrix(port);
[data_diff] = remo_snget_matrix(port);
data_single_voltage = single(data_single) ./ 1024 .*2.56;
data_diff_voltage = single(data_diff) ./ 512 .*2.56;

plot(data_single_voltage, 'b');
plot(data_diff_voltage, 'g');
legend('single voltage','diff voltage');
hold off;