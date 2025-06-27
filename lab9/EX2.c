#include "c4mlib.h"

void PWM_init();

int main()
{
    C4M_DEVICE_set();

    PWM_init();
    sei();

    printf("start\n");
    while (1)
    {
        ;
    }
    return 0;
}

void PWM_init()
{
    // PORTB 5:6 output
    REGFPT(&DDRB, 0x60, 5, 0x03);

    // Clear OC1A/OC1B on compare match
    REGFPT(&TCCR1A, 0xc0, COM1A0, 0x02);
    REGFPT(&TCCR1A, 0x30, COM1B0, 0x02);

    // PWM, Phase and Frequency Correct TOP-> ICRn
    REGFPT(&TCCR1A, 0x03, WGM10, 0x00);
    REGFPT(&TCCR1B, 0x18, WGM12, 0x02);

    // clk/64
    REGFPT(&TCCR1B, 0x07, CS10, 0x03);

    // set up TOP : ICRn
    ICR1 = 863;

    // set up channel 1A and 1B value
    OCR1A = 863 * 0.33;
    OCR1B = 863 * 0.66;
}