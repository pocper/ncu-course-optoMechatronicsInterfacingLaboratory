#include "c4mlib.h"

void ADC_init();

int main()
{
    C4M_DEVICE_set();

    ADC_init();
    sei();

    while (1)
    {
        REGFPT(&ADCSRA, 0x40, 6, 1);

        _delay_ms(100);
        // 0.5 [V] -> -640[G]
        // 2.5 [V] ->    0[G]
        // 4.5 [V] ->  640[G]

        printf("%f[G]\n", ((ADC / 1024.) * 5 - 0.5) / 4 * 1280 - 640);
    }
    return 0;
}

void ADC_init()
{
    //設定外部參考電壓
    REGFPT(&ADMUX, 0xC0, 6, 1);

    //設定10位元轉換靠右
    REGFPT(&ADMUX, 0x20, 5, 0);

    //設定非連續或觸發轉換
    REGFPT(&ADCSRA, 0x20, 5, DISABLE);

    //設定ADC1 F1輸入
    REGFPT(&DDRF, 0x02, 0, 0);

    //設定致能ADC
    REGFPT(&ADCSRA, 0x80, 7, ENABLE);

    //設定ADC時脈 clk/128
    REGFPT(&ADCSRA, 0x07, 0, 7);

    //設定ADC1單通道
    REGFPT(&ADMUX, 0x1f, 0, 1);
}