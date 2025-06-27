
/*
 * Matlab -> v3 -> DAC
 * v3 -> ADC -> Matab -> plot
*/

#include "c4mlib.h"

#define arrSize 200

void ADCPostPro_step(void *VoidStr_p);
void SPIDACTrm_step(void *VoidStr_p);

void ADC_init();
void SPI_init();
void timer3_init();

typedef struct
{
    uint16_t *InData_p;         /*Pointer points to the Input Data Source */
    uint16_t *DataList_p;       /* Pointer points to the buffer array */
    uint8_t DataLength;         /* Length of Datalist */
    uint8_t DataCount;          /*Data Count of the data in list*/
    uint8_t TaskId;             //The TaskId got after registered
    uint8_t NextTaskNum;        //Number of Next Task
    uint8_t *NextTask_p;        //pointer to the List of TaskId for NextTasks
    volatile uint8_t TrigCount; //Triggered Counter
} ADCPostProStr_t;

typedef struct
{
    uint16_t OutData;
    uint16_t *OutData_p;        /*Pointer points to the Output Data Source */
    uint16_t *DataList_p;       /* Pointer points to the Out buffer array */
    uint8_t DataLength;         /* Length of Datalist */
    uint8_t DataCount;          /*Data Count of the data in list*/
    uint8_t TaskId;             //The TaskId got after registered
    uint8_t NextTaskNum;        //Number of Next Task
    uint8_t *NextTask_p;        //pointer to the List of TaskId for NextTasks
    volatile uint8_t TrigCount; //Triggered Counter
} DACPreProStr_t;

typedef struct
{
    uint8_t Mode;     /* Transmit Mode*/
    uint8_t CardId;   /* Card Identification Number */
    uint8_t RegAdd;   /* Register Id of the register of this Card*/
    uint8_t Bytes;    //Bytes of the Data to Transmit;
    uint16_t *Data_p; //Data to be Transmit
    uint8_t Counter;
    uint8_t TaskId;             //The TaskId got after registered
    uint8_t NextTaskNum;        //Number of Next Task
    uint8_t *NextTask_p;        //pointer to the List of TaskId for NextTasks
    volatile uint8_t TrigCount; //Triggered Counter
} SpiDacTrmStr_t;

#define ADCPOSTPRO_LAY(ADCPostPro_str, ListNum, _NextTaskNum, InDataAdd) \
    uint16_t ADCPostPro_str##_DataList[ListNum];                         \
    uint8_t ADCPostPro_str##_NextTaskList[_NextTaskNum];                 \
    ADCPostProStr_t ADCPostPro_str =                                     \
        {                                                                \
            .InData_p = InDataAdd,                                       \
            .DataList_p = ADCPostPro_str##_DataList,                     \
            .DataLength = ListNum,                                       \
            .DataCount = 0,                                              \
            .TaskId = 0,                                                 \
            .NextTaskNum = _NextTaskNum,                                 \
            .NextTask_p = ADCPostPro_str##_NextTaskList,                 \
            .TrigCount = 0}

#define DACPrePro_LAY(DACPreProStr, ListNum, _NextTaskNum) \
    uint16_t DACPreProStr_DataList[ListNum];               \
    uint8_t DACPreProStr_NextTaskList[_NextTaskNum];       \
    DACPreProStr_t DACPreProStr = {                        \
        .DataList_p = DACPreProStr_DataList,               \
        .DataLength = ListNum,                             \
        .DataCount = 0,                                    \
        .TaskId = 0,                                       \
        .OutData_p = 0,                                    \
        .NextTaskNum = _NextTaskNum,                       \
        .NextTask_p = DACPreProStr_NextTaskList}

#define SPIDACTrm_LAY(SpiDacTrmStr, _NextTaskNum)                                                \
    uint8_t SpiDacTrmStr_NextTaskList[1] = {0};                                                  \
    SpiDacTrmStr_t SpiDacTrmStr = {                                                              \
        .Mode = 2,    /* Transmit Mode*/                                                         \
        .CardId = 1,  /* Card Identification Number */                                           \
        .RegAdd = 80, /*參考 操作控制參數快查表 */                                    \
        .Bytes = 2,   /*參考 操作控制參數快查表 */                                    \
        .Data_p = 0,  /*Data to be Transmit */                                                   \
        .Counter = 0,                                                                            \
        .TaskId = 0,                             /*The TaskId got after registered */            \
        .NextTaskNum = _NextTaskNum,             /*Number of Next Task */                        \
        .NextTask_p = SpiDacTrmStr_NextTaskList, /*pointer to the List of TaskId for NextTasks*/ \
        .TrigCount = 0                           /*Triggered Counter */                          \
    }

int main()
{
    C4M_DEVICE_set();

    //設定Timer
    TIM3_HW_LAY();
    hardware_set(&TIM1_3HWSet_str);

    //設定Timer3中斷
    TIMHWINT_LAY(TIMINT_Str, 3, 1);
    timer3_init();

    //設定ADC
    ADC_HW_LAY();
    hardware_set(&ADCHWSet_str);
    ADC_init();

    //設定ADC中斷
    ADCHWINT_LAY(ADCHWINT_Str, 0, 2);
    hardware_set(&ADCHWINT_Str);

    //設定SPI
    SPI_init();

    unsigned int ADC_readData;

    ADCPOSTPRO_LAY(ADC_PostPro_Str, arrSize, 0, &ADC_readData);
    DACPrePro_LAY(DAC_PostPro_Str, arrSize, 0);
    SPIDACTrm_LAY(SPI_DAC_Str, 0);

    HMI_snget_matrix(HMI_TYPE_UI16, 1, DAC_PostPro_Str.DataLength, DAC_PostPro_Str.DataList_p);
    SPI_DAC_Str.Data_p = DAC_PostPro_Str.DataList_p;

    //設定降頻器配置
    // int period[2];
    // for (int i = 0; i < sizeof(period) / sizeof(int); i++)
    //     period[i] = 2000;
    int period = 26;

    FREQREDU_LAY(FreqRedu_Str, 1, 1, &OCR3A, 2, &period);

    int8_t freq_TaskID;
    freq_TaskID = FreqRedu_reg(&FreqRedu_Str, &SPIDACTrm_step, &SPI_DAC_Str, 1, 0);

    FreqRedu_en(&FreqRedu_Str, freq_TaskID, ENABLE);

    uint8_t TaskID[3];

    //將降頻器登入進Timer中斷
    TaskID[0] = HWInt_reg(&TIMINT_Str, &FreqRedu_step, &FreqRedu_Str);
    HWInt_en(&TIMINT_Str, TaskID[0], ENABLE);

    //將ADC結果登入進ADC中斷
    RT_REG_IO_LAY(ADC_result_Str, 0, &ADCL, 2, (uint8_t *)&ADC_readData);
    TaskID[1] = HWInt_reg(&ADCHWINT_Str, &RealTimeRegGet_step, &ADC_result_Str);
    HWInt_en(&ADCHWINT_Str, TaskID[1], ENABLE);

    //將ADC_postPro_step結果登入進ADC中斷
    TaskID[2] = HWInt_reg(&ADCHWINT_Str, &ADCPostPro_step, &ADC_PostPro_Str);
    HWInt_en(&ADCHWINT_Str, TaskID[2], ENABLE);

    sei();

    while (1)
    {
        ;
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

    //設定致能ADC Interrupt
    REGFPT(&ADCSRA, 0x08, 3, ENABLE);

    //設定ADC時脈 clk/128
    REGFPT(&ADCSRA, 0x07, 0, 7);

    //設定ADC1單通道
    REGFPT(&ADMUX, 0x1f, 0, 1);
}

void SPI_init()
{
    REGFPT(&DDRF, 0x01, 0, 0x01); //F1為MCP4922晶片選擇
    REGFPT(&DDRB, 0x07, 0, 0x07); //設定PB1(SCK) / PB2(MOSI)為輸出

    //主板設定
    REGFPT(&SPSR, 0x01, SPI2X, 1); //設定雙倍工作時脈
    REGFPT(&SPCR, 0x03, SPR0, 0);  //SPI_FreqDivide_4
    REGFPT(&SPCR, 0x04, CPHA, 0);  //前收後送
    REGFPT(&SPCR, 0x08, CPOL, 0);  //設定前緣為上
    REGFPT(&SPCR, 0x10, MSTR, 1);  //設定為主板
    REGFPT(&SPCR, 0x20, DORD, 0);  //高位元先送
    REGFPT(&SPCR, 0x40, SPE, 1);   //SPI致能
}

void timer3_init()
{
    //normal mode
    REGFPT(&TCCR3A, 0x03, 0, 0);
    //normal mode
    REGFPT(&TCCR3B, 0x18, 3, 1);
    //設定timer時脈 clk/1024
    REGFPT(&TCCR3B, 0x07, 0, 5);

    OCR3A = 269;
    //設定timer3A致能
    REGFPT(&ETIMSK, 0x10, 4, 1);
}

void ADCPostPro_step(void *VoidStr_p)
{
    volatile ADCPostProStr_t *Str_p = (ADCPostProStr_t *)VoidStr_p;

    Str_p->DataList_p[Str_p->DataCount] = *(Str_p->InData_p);

    if ((Str_p->DataCount + 1) == (Str_p->DataLength))
    {
        cli();
        HMI_snput_matrix(HMI_TYPE_UI16, 1, Str_p->DataLength, Str_p->DataList_p);
        Str_p->DataCount = 0;
        sei();
    }
    else
        Str_p->DataCount++;
}

void SPIDACTrm_step(void *VoidStr_p)
{
    SpiDacTrmStr_t *Str_p = (SpiDacTrmStr_t *)VoidStr_p; /*Typeset Structure pointer*/

    REGFPT(&PORTF, 0x01, 0, 0);
    ASA_SPIM_trm(Str_p->Mode, Str_p->CardId, Str_p->RegAdd, Str_p->Bytes, Str_p->Data_p + Str_p->Counter, 0);
    REGFPT(&PORTF, 0x01, 0, 1);

    //觸發ADC轉換
    REGFPT(&ADCSRA, 0x40, 6, 1);

    if ((Str_p->Counter + 1) < arrSize)
        Str_p->Counter++;
    else
        Str_p->Counter = 0;
}