#include "c4mlib.h"

#define ADC0_FG_DATA_INT                \
    {                                   \
        .DDx0_7 = INPUT,                \
        .MUXn0_4 = ADC_SINGLE_END_0_X1, \
        .REFSn0_1 = ADC_REF_AREF,       \
        .ADLARn = ADC_RES_10BITS,       \
        .ADPSn0_2 = ADC_CLK_DIV_BY2,    \
        .ADFRn = DISABLE,               \
        .ADENn = ENABLE,                \
        .ADIEn = ENABLE,                \
        .Total = 8                      \
    }

int main()
{
    C4M_STDIO_init();
    //設定內部2.56V
    REGFPT(&ADMUX, 0xC0, 6, 3);

    //設定10位元轉換
    REGFPT(&ADMUX, 0x20, 5, ADC_RES_10BITS);

    //設定非連續或觸發轉換
    REGFPT(&ADCSRA, 0x20, 5, DISABLE);

    //設定ADC接角 F0~4皆輸入
    REGFPT(&DDRF, 0x0f, 0, 0);

    //設定輸入訊號來源
    REGFPT(&ADMUX, 0x1f, 0, ADC_SINGLE_END_0_X1);

    //設定致能ADC
    REGFPT(&ADCSRA, 0x80, 7, ENABLE);

    //設定工作時脈除頻
    REGFPT(&ADCSRA, 0x07, 0, ADC_CLK_DIV_BY2);

    int isContinued = 1;
    int isDoneConverted;
    int data0, data1;
    char temp[3];

    printf("start while-loop\n");

    while (isContinued)
    {
        printf("Set input voltage 0V\n");
        scanf("%s", temp);
        printf("temp=%s\n", temp);

        //觸發ADC轉換
        REGFPT(&ADCSRA, 0x40, 6, 1);

        do
        {
            _delay_ms(100);
            REGFGT(&ADCSRA, 0x10, 4, &isDoneConverted);
        } while (!isDoneConverted);

        REGGET(&ADCL, 2, &data0);
        printf("%d\n", data0);

        printf("Set input voltage 1.23V\n");
        scanf("%s", temp);
        printf("temp=%s\n", temp);

        //觸發ADC轉換
        REGFPT(&ADCSRA, 0x40, 6, 1);

        do
        {
            _delay_ms(100);
            REGFGT(&ADCSRA, 0x10, 4, &isDoneConverted);
        } while (!isDoneConverted);

        REGGET(&ADCL, 2, &data1);
        printf("%d\n", data1);

        printf("Is continuing detect ADC? (1/0)\n");
        scanf("%d", &isContinued);
    }

    return 0;
}