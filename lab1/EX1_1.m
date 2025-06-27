clear; clc; close;

load sin.mat;
load cos.mat;
load output_sin.mat;
load output_cos.mat;

hold on;
t = linspace(-0.5, 0.5, 200);
%sin
plot(t, sin);
plot(t, output_sin);
%cos
% plot(t, cos);
% plot(t, output_cos);

xlabel('時間[s]');
ylabel('軸度');

legend('輸入', '輸出')

hold off;
%sin
saveas(gcf, 'EX1A', 'jpeg');
%cos
% saveas(gcf, 'EX1B', 'jpeg');
