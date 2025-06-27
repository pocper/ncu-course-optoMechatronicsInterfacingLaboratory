clear;clc;close;

[port] = remo_open(8);

[msg] = remo_get_msg(port);
disp(msg);%start while-loop
ans = 1;

while ans
    [msg] = remo_get_msg(port);
    disp(msg);%Set input voltage 0V

    temp = input("pressed any key\n");
    remo_put_msg(port,temp);
    
    [msg] = remo_get_msg(port);
    disp(msg);%temp=

    [msg] = remo_get_msg(port);
    Code_0V = str2num(msg);
    disp(msg);%Data=

    [msg] = remo_get_msg(port);
    disp(msg);%Set input voltage 1.23V

    temp = input("Press any key\n");
    remo_put_msg(port,temp);
    
    [msg] = remo_get_msg(port);
    disp(msg);%temp=
    
    [msg] = remo_get_msg(port);
    Code_1_23V = str2num(msg);
    disp(msg);%Data=

    disp("Offset= "+num2str(Code_0V));
    disp("Gain= "+num2str(1.23/(Code_1_23V-Code_0V)));
    
    
    [msg] = remo_get_msg(port);
    disp(msg);%Is continuing detect ADC? (1/0)

    ans = input("Enter:");
    remo_put_msg(port,ans);
    ans = str2num(ans);
end

remo_close(port);
