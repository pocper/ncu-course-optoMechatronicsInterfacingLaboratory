clear all;
close;
t = linspace(-0.5, 0.5, 300);

sin = single(sin(50 * t));

sin_1 = sin(1:255);
sin_2 = sin(256:300);

save('sin_1', 'sin_1');
save('sin_2', 'sin_2');
