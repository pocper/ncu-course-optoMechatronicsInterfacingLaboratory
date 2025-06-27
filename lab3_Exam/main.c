#include "c4mlib.h"
#define TMP121_Mode 3
#define MAX7219_Mode 6
#define TMP121_ID 8
#define MAX7219_ID 9
#define TempRegister 0x00
#define WaitTick 1

#define Decode 0x09    // 9
#define Intensity 0x0a //10
#define Scan 0x0b      //11
#define ShutDown 0x0c  //12
#define Test 0x0f      //15

void SetUp();
void Flash();
void Display(int, float);
void MAX7219(char, char);
float TempSensor();
int Index(int);
float Bin2Dec(uint8_t, uint8_t, uint8_t);

int main()
{
    C4M_STDIO_init();
    SetUp();

    printf("-----start-----\n");

    int integer;
    float Temp, decimal;

    while (1)
    {
        Flash();
        Temp = TempSensor();
        integer = Temp;
        decimal = Temp - integer;
        Display(integer, decimal);
        _delay_ms(1000);
    }
    return 0;
}

void SetUp()
{
    //主板設定
    SPI_fpt(&SPCR, 0x40, 6, 1); //SPI致能
    SPI_fpt(&SPSR, 0x01, 0, 1); //設定雙倍工作時脈
    SPI_fpt(&SPCR, 0x03, 0, 0); //SPI_FreqDivide_4
    SPI_fpt(&SPCR, 0x04, 2, 0); //前收後送
    SPI_fpt(&SPCR, 0x08, 3, 0); //設定前緣為上
    SPI_fpt(&SPCR, 0x10, 4, 1); //設定為主板
    SPI_fpt(&SPCR, 0x20, 5, 0); //高位元先送

    DDRB = 0x06; //設定MISO為輸入,MOSI為輸出,SCK為輸出
    DDRF = 0x03; //F0為TMP121住址選擇,F1為MAX7219住址選擇

    //MAX7219
    MAX7219(Decode, 0x0);
    MAX7219(Intensity, 0x07);
    MAX7219(ShutDown, 0x01);
}

void Flash()
{
    for (int i = 1; i < 9; i++)
    {
        MAX7219(i, 0);
    }
    _delay_ms(100);
}

void Display(int integer, float decimal)
{
    char BCD[10] = {0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b};
    int num[8] = {0};
    int len[2] = {0};
    int j = 0, digits = 0;
    int temp;
    int DecimalPoint;

    //判斷位數

    //整數位數
    temp = integer;
    while (temp)
    {
        temp /= 10;
        digits++;
    }

    len[0] = digits;
    DecimalPoint = digits - 1; //小數點新增位置

    //小數位數
    digits += 4;
    len[1] = 4;

    MAX7219(Scan, digits - 1); //設定掃描位數
    //輸出數字

    //整數處理
    for (int i = len[0] - 1; i > -1; i--)
    {
        num[j] = (integer / Index(i)) % 10;
        j++;
    }

    //小數處理

    temp = decimal * Index(len[1]);

    for (int i = len[1] - 1; i > -1; i--)
    {
        num[j] = (temp / Index(i)) % 10;
        j++;
    }

    for (int i = 0; i < digits; i++)
    {
        if (i == DecimalPoint)
        {
            MAX7219(i + 1, BCD[num[i]] + 128);
        }
        else
        {
            MAX7219(i + 1, BCD[num[i]]);
        }
    }
}

void MAX7219(char REG, char data)
{
    uint8_t temp = data;
    PORTF = 0x01;
    ASA_SPIM_trm(MAX7219_Mode, MAX7219_ID, REG, 1, &temp, WaitTick);
    PORTF = 0x03;
}

float TempSensor()
{
    uint16_t Temp = 0;

    PORTF = 0x02;
    ASA_SPIM_rec(TMP121_Mode, TMP121_ID, TempRegister, 2, &Temp, WaitTick);
    PORTF = 0x03;
    printf("Temperature:%.4f\n", Bin2Dec((Temp & 0x8000) >> 15, (Temp & 0x7f80) >> 7, (Temp & 0x78) >> 3));
    _delay_ms(300); //讀取溫度運行時間
    return Bin2Dec((Temp & 0x8000) >> 15, (Temp & 0x7f80) >> 7, (Temp & 0x78) >> 3);
}

int Index(int len)
{
    int sum = 1;
    for (int i = 0; i < len; i++)
    {
        sum *= 10;
    }

    return sum;
}

float Bin2Dec(uint8_t sign, uint8_t num1, uint8_t num2)
{
    int integer = 0;
    float point = 0;

    for (int i = 7; i > -1; i--)
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

    if (sign)
    {
        return (integer + point) - 256;
    }
    else
    {
        return (integer + point);
    }
}