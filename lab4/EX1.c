#include "c4mlib.h"
#define N 2

void output_High();
void output_Low();
void timer3_init();

int main()
{
    C4M_DEVICE_set();
    DDRD = 0x01;

    //設定Timer
    TIM3_HW_LAY();

    hardware_set(&TIM1_3HWSet_str);

    //設定Timer中斷
    TIMHWINT_LAY(Tim3, 3, 1);

    timer3_init();

    //設定降頻器配置
    uint16_t period[2 * N] = {1000, 2000, 3000, 4000};

    FREQREDU_LAY(redu, 2 * N, 2 * N, &OCR3A, 2, period);

    uint8_t ID[4];

    ID[0] = FreqRedu_reg(&redu, &output_High, NULL, 1, 0);
    ID[1] = FreqRedu_reg(&redu, &output_Low, NULL, 1, 1);

    ID[2] = FreqRedu_reg(&redu, &output_High, NULL, 1, 2);
    ID[3] = FreqRedu_reg(&redu, &output_Low, NULL, 1, 3);

    FreqRedu_en(&redu, ID[0], 1);
    FreqRedu_en(&redu, ID[1], 1);
    FreqRedu_en(&redu, ID[2], 1);
    FreqRedu_en(&redu, ID[3], 1);

    uint8_t TaskID = HWInt_reg(&Tim3, &FreqRedu_step, &redu);
    HWInt_en(&Tim3, TaskID, 1);
    sei();

    while (1)
    {
        ;
    }

    return 0;
}

void timer3_init()
{
    REGFPT(&TCCR3A, 0x03, 0, 0);
    REGFPT(&TCCR3B, 0x18, 3, 1);
    REGFPT(&TCCR3B, 0x07, 0, 5);
    OCR3A = 188;
    REGFPT(&ETIMSK, 0x10, 4, 1);
}

void output_High()
{
    REGFPT(&PORTD, 0x01, 0, 0x01);
}

void output_Low()
{
    REGFPT(&PORTD, 0x01, 0, 0);
}
