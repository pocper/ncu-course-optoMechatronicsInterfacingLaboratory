#include "c4mlib.h"

void PWM_init();

ISR(TIMER2_OVF_vect)
{
    REGFPT(&PORTB, 0x40, 6, 1);
    printf("OVERFLOW\n");
    REGFPT(&PORTB, 0x40, 6, 0);
}

int main()
{
    C4M_DEVICE_set();
    PWM_init();
    sei();
    while (1)
    {
        ;
    }
    return 0;
}

void PWM_init()
{
    REGFPT(&TCCR2, 0x48, 3, 9); //Fast PWM
    REGFPT(&TCCR2, 0x07, 0, 4); //除頻值1024
    REGFPT(&TCCR2, 0x30, 4, 3); //正脈波
    REGFPT(&TIMSK, 0x40, 6, 1); //Overflow Flag中斷致能
    OCR2 = 107;
    DDRB = (DDRB & (~(0xc0))) | 0xc0; //PB6 觸發腳位 || PB7 OC2
}