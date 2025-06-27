clear;clc;close;

[port] = remo_open(8);
title('幅值頻譜');
xlabel('頻率[Hz]');
ylabel('幅值');
ylim([0 ,0.6]);

Fs = 2700; %取樣頻率
T = 1 / Fs;
L = 40; %訊號長度
N = 2^nextpow2(L); %取樣點數
f = Fs / L * (0:L-1);

while 1
    [data_single] = remo_snget_matrix(port);
    [data_diff] = remo_snget_matrix(port);
    data_single_voltage = single(data_single) ./ 1024 .*2.56;
    data_diff_voltage = single(data_diff) ./ 512 .*2.56;

    Y1 = fft(data_single_voltage) / L * 2;
    Y2 = fft(data_diff_voltage) / L * 2;
    
    A1 = abs(Y1);
    A2 = abs(Y2);
    
    subplot(1,2,1);
    plot(f, A1);
    title('single');
    xlabel('頻率(Hz)');
    ylabel('幅值');
    
    subplot(1,2,2);
    plot(f, A2);
    title('diff');
    xlabel('頻率(Hz)');
    ylabel('幅值');
end