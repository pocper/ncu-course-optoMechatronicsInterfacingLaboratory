#include "c4mlib.h"
#define SLAADD 0x48
#define Mode 5
#define WAITTICK 50
#define TempRegistor 0x00
#define ConfigRegistor 0x01
#define T_low_Registor 0x02
#define T_high_Registor 0x03

void SetUp();
float Bin2Dec(uint8_t, uint8_t);

char TempStatus = 0;
uint8_t Temperature[2] = {0, 0};
float temperature = 0;

int main()
{

    C4M_STDIO_init();
    SetUp();
    printf("-----start-----\n");

    while (1)
    {
        TWIM_rec(Mode, SLAADD, TempRegistor, 2, Temperature, WAITTICK); //接收溫度
        temperature = Bin2Dec(Temperature[0], Temperature[1] >> 4);
        HMI_snput_matrix(8, 1, 1, &temperature); //送出溫度資料到matlab
        // printf("Temperature : %.4f degree\n", temperature);

        if (TempStatus == 1)
        {
            // printf("Temperature too HIGH(OVER 27 Degree)!\n");
            DIO_fpt(&PORTB, 0x06, 0, 0x02);
        }
        else if (TempStatus == 2)
        {
            TempStatus = 0;
            // printf("Temperature too LOW(LOWER 26 Degree)!\n");
            DIO_fpt(&PORTB, 0x06, 0, 0x04);
        }
        _delay_ms(1000);
    }

    return 0;
}

ISR(INT2_vect)
{
    if (TempStatus == 0)
        TempStatus++;
    else
        TempStatus++;
}

void SetUp()
{
    //中斷登入設定
    EICRA = 0x30; //設定上升緣觸發
    DDRD = 0xfb;  //PD2輸入
    EIMSK = 0x04; //INT2中斷啟用
    sei();

    //通訊設定
    TWI_fpt(&TWSR, 0x03, 0, 0);      //TWPS=DIV_BY_1
    TWI_fpt(&TWCR, 0x01, 0, 1);      //致能TWI通訊
    TWI_fpt(&TWCR, 0x45, 0, 0x45);   //致能TWI ACK致能
    TWI_fpt(&TWAR, 0xfe, 1, SLAADD); //設定TWI編號(slave of this card)
    TWI_fpt(&TWAR, 0x01, 0, 0);      //設定廣播關閉
    TWI_fpt(&TWBR, 0xff, 0, 12);     //設定工作時脈 276.48KHz

    uint8_t zero = 0, one = 1, third = 3;
    uint8_t ThresholdTemp[2] = {26, 27};
    TWIM_ftm(Mode, SLAADD, ConfigRegistor, 0x01, 0, &zero, WAITTICK);        //設定連續感測模式
    TWIM_ftm(Mode, SLAADD, ConfigRegistor, 0x02, 1, &one, WAITTICK);         //設定溫度量測模式
    TWIM_ftm(Mode, SLAADD, ConfigRegistor, 0x04, 2, &one, WAITTICK);         //設定警告訊號為HI
    TWIM_ftm(Mode, SLAADD, ConfigRegistor, 0x60, 5, &third, WAITTICK);       //設定精度為0.0625度C
    TWIM_trm(Mode, SLAADD, T_low_Registor, 2, &ThresholdTemp[0], WAITTICK);  //設定低溫門檻為26度
    TWIM_trm(Mode, SLAADD, T_high_Registor, 2, &ThresholdTemp[1], WAITTICK); //設定高溫門檻為27度

    //LED 送出設定
    DIO_fpt(&DDRB, 0x06, 0, 0xff); //portA全輸出
}

float Bin2Dec(uint8_t num1, uint8_t num2)
{
    int integer = 0;
    float point = 0;

    for (int i = 6; i > -1; i--)
    {
        integer += (num1 >> i) & 1;
        if (i)
            integer *= 2;
    }

    for (int i = 0; i < 4; i++)
    {
        point += (num2 >> i) & 1;
        point /= 2;
    }

    if (num1 / 128)
    {
        return (integer + point) - 128;
    }
    else
    {
        return (integer + point);
    }
}