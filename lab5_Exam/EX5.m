clear;clc;close;
[port] = remo_open(8);

Fs = 2201;                        %取樣頻率
DataLength = 200;                 %訊號長度

f = Fs / DataLength * (0:DataLength-1);

%產出sin波送給v3
t = linspace(0,2*pi,DataLength);
y = sin(1000 * t);
y = uint16((y + 1) / 2 * (2^12 -1));
remo_snput_matrix(port,y);

while 1
    [data] = remo_snget_matrix(port);
    data_volt = single(data) ./1024 .* 4.30;
    time = linspace(0,1,DataLength);
    subplot(1,2,1);
    plot(time, data_volt);
    ylim([0,4.30]);
    xlabel('時間[s]');
    ylabel('電壓[V]');
    title('DAC波型 | A_{ref} 4.30V');
    
    subplot(1,2,2);
    Y = fft(data);
    plot(f, abs(Y));
    ylim([0,500]);
    xlabel('頻率[Hz]');
    ylabel('幅值');
    title('幅值頻譜');
end


remo_close(port);