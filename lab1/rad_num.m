clear all;
close;
t = linspace(-0.5, 0.5, 200);

sin = single(sin(30 * t));
save('sin', 'sin');

cos = single(cos(60 * t + 2));
save('cos', 'cos');
