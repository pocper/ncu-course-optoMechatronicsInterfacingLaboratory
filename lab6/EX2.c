#include "c4mlib.h"

void ADC_init();
void ADCPostPro_step(void *VoidStr_p);
void PWMPrePro_step(void *VoidStr_p);

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
} PwmPreProStr_t;

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

#define PwmPrePro_LAY(PwmPreProStr, ListNum, _NextTaskNum) \
    uint16_t PwmPreProStr##_DataList[ListNum];             \
    uint8_t PwmPreProStr##_NextTaskList[_NextTaskNum];     \
    PwmPreProStr_t PwmPreProStr = {                        \
        .DataList_p = PwmPreProStr##_DataList,             \
        .DataLength = ListNum,                             \
        .DataCount = 0,                                    \
        .TaskId = 0,                                       \
        .NextTaskNum = _NextTaskNum,                       \
        .NextTask_p = PwmPreProStr##_NextTaskList}

int main()
{
    C4M_DEVICE_set();

    DDRB = 0xff;

    //設定Timer
    TIM3_HW_LAY();
    hardware_set(&TIM1_3HWSet_str);

    //設定Timer3中斷
    TIMHWINT_LAY(TIMINT_Str, 3, 1);
    timer3_init();

    //設定PWM
    PWM2_HW_LAY();
    hardware_set(&PWM2HWSet_str);

    //設定PWM中斷
    PWMHWINT_LAY(PwmInt_Str, 0, 1);
    hardware_set(&PwmInt_Str);

    //設定ADC
    ADC_HW_LAY();
    hardware_set(&ADCHWSet_str);
    ADC_init();

    //設定ADC中斷
    ADCHWINT_LAY(ADCHWINT_Str, 0, 2);
    hardware_set(&ADCHWINT_Str);

    uint16_t ADC_readData;
    ADCPOSTPRO_LAY(ADCPostPro_Str, 200, 1, &ADC_readData);
    PwmPrePro_LAY(PwmPrePro_Str, 200, 1);

    int period[2];
    for (int i = 0; i < sizeof(period) / sizeof(int); i++)
        period[i] = 100;

    FREQREDU_LAY(PWMFreqRedu_Str, 2, 2, &OCR3A, 2, &period);
    int8_t freq_TaskID[2];
    uint8_t one = 1;
    RT_FLAG_IO_LAY(ADCTrig_Str, 0, &ADCSRA, 0x40, 6, &one);
    freq_TaskID[0] = FreqRedu_reg(&PWMFreqRedu_Str, &RealTimeFlagPut_step, &ADCTrig_Str, 1, 1);

    RT_REG_IO_LAY(PWMSend_Str, 0, &PORTB, 1, &PwmPrePro_Str.OutData);
    freq_TaskID[1] = FreqRedu_reg(&PWMFreqRedu_Str, &RealTimeRegPut_step, &PWMSend_Str, 1, 1);

    FreqRedu_en(&PWMFreqRedu_Str, freq_TaskID[0], ENABLE);
    FreqRedu_en(&PWMFreqRedu_Str, freq_TaskID[1], ENABLE);

    //設定Pipeline
    PIPELINE_LAY(2, 4, 10);
    //PWM訊號準備
    Pipeline_reg(&SysPipeline_str, &PWMPrePro_step, &PwmPrePro_Str, NULL);
    Pipeline_reg(&SysPipeline_str, &ADCPostPro_step, &ADCPostPro_Str, NULL);

    uint8_t TaskID[3];

    //將降頻器登入進Timer中斷
    TaskID[0] = HWInt_reg(&TIMINT_Str, &FreqRedu_step, &PWMFreqRedu_Str);
    HWInt_en(&TIMINT_Str, TaskID[0], ENABLE);

    //將ADC結果登入進ADC中斷
    RT_REG_IO_LAY(ADCRead_Str, 1, &ADCL, 2, (uint8_t *)&ADC_readData);
    TaskID[1] = HWInt_reg(&ADCHWINT_Str, &RealTimeRegGet_step, &ADCRead_Str);
    HWInt_en(&ADCHWINT_Str, TaskID[1], ENABLE);

    //將pipeline登入進ADC中斷
    TaskID[2] = HWInt_reg(&ADCHWINT_Str, &Pipeline_step, &SysPipeline_str);
    HWInt_en(&ADCHWINT_Str, TaskID[2], ENABLE);

    sei();
    TRIG_NEXT_TASK(0);

    HMI_snget_matrix(HMI_TYPE_UI16, 1, PwmPrePro_Str.DataLength, PwmPrePro_Str.DataList_p);

    while (1)
    {
        ;
    }
    return 0;
}

void ADC_init()
{
    //設定內部2.56V
    REGFPT(&ADMUX, 0xC0, 6, 3);

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

    //設定ADC1 and GND
    REGFPT(&ADMUX, 0x1f, 0, 1);
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

    // freq = 11059200 / (2 * 1024 * (1 + 100)) = 53.46
}

void PWMPrePro_step(void *VoidStr_p)
{
    volatile PwmPreProStr_t *Str_p = (PwmPreProStr_t *)VoidStr_p;

    if (Str_p->DataCount == Str_p->DataLength)
    {
        Str_p->DataCount = 0;
    }

    //PB6輸出
    Str_p->OutData = Str_p->DataList_p[Str_p->DataCount] << 6;
    Str_p->DataCount++;
    TRIG_NEXT_TASK(1); /* pipeline task trigger*/
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
    TRIG_NEXT_TASK(0);
}
