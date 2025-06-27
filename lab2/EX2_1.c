#include "c4mlib.h"
#define SLAADD 0x48
#define Mode 5
#define WAITTICK 50
#define TempRegistor 0x00
#define ConfigRegistor 0x01

void SetUp();
float Bin2Dec(uint8_t, uint8_t);

int main()
{
    C4M_STDIO_init();
    SetUp();

    char ans;
    uint8_t one = 1, Temperature[2] = {0, 0};

    while (1)
    {
        printf("Read Temperature(Y/N):\n");
        scanf("%s", &ans);
        if (ans == 'Y' || ans == 'y')
        {
            ans = 'N';
            TWIM_ftm(Mode, SLAADD, ConfigRegistor, 0x80, 7, &one, WAITTICK); //設定單發模式
            TWIM_rec(Mode, SLAADD, TempRegistor, 2, Temperature, WAITTICK);  //接收溫度
            printf("Temperature:%.4f\n\n", Bin2Dec(Temperature[0], Temperature[1] >> 4));
        }
    }

    return 0;
}

void SetUp()
{
    //通訊設定
    TWI_fpt(&TWSR, 0x03, 0, 0);      //TWPS=DIV_BY_1
    TWI_fpt(&TWCR, 0x01, 0, 1);      //致能TWI通訊
    TWI_fpt(&TWCR, 0x45, 0, 0x45);   //致能TWI ACK致能
    TWI_fpt(&TWAR, 0xfe, 1, SLAADD); //設定TWI編號(slave of this card)
    TWI_fpt(&TWAR, 0x01, 0, 0);      //設定廣播關閉
    TWI_fpt(&TWBR, 0xff, 0, 12);     //設定工作時脈 276.48KHz

    uint8_t one = 1, third = 3;
    TWIM_ftm(Mode, SLAADD, ConfigRegistor, 0x01, 0, &one, WAITTICK);   //設定休眠模式
    TWIM_ftm(Mode, SLAADD, ConfigRegistor, 0x60, 5, &third, WAITTICK); //設定精度為0.0625度C
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