#include "c4mlib.h"

void ADC_init();
void ADC0PostPro_step(void *VoidStr_p);
void ADC1PostPro_step(void *VoidStr_p);

void PWMPrePro_step(void *VoidStr_p);

void timer0_init();
void timer1_init();
void timer2_init();

typedef struct
{
    uint16_t *InData_p;         /*Pointer points to the Input Data Source */
    uint16_t *DataList_p;       /* Pointer points to the buffer array */
    uint8_t DataLength;         /* Length of Datalist */
    uint8_t DataCount;          /*Data Count of the data in list*/
    uint8_t TaskId;             // The TaskId got after registered
    uint8_t NextTaskNum;        // Number of Next Task
    uint8_t *NextTask_p;        // pointer to the List of TaskId for NextTasks
    volatile uint8_t TrigCount; // Triggered Counter
} ADCPostProStr_t;

typedef struct
{
    uint16_t OutData;
    uint16_t *OutData_p;  /*Pointer points to the Output Data Source */
    uint16_t *DataList_p; /* Pointer points to the Out buffer array */
    uint8_t DataLength;   /* Length of Datalist */
    uint8_t DataCount;    /*Data Count of the data in list*/
    uint8_t channel_num;
    uint16_t *OutArrList_p;
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

#define PwmPrePro_LAY(PwmPreProStr, ListNum, _channel_num) \
    uint16_t PwmPreProStr##_ArrayList[_channel_num];       \
    uint16_t PwmPreProStr##_DataList[ListNum];             \
    PwmPreProStr_t PwmPreProStr = {                        \
        .DataList_p = PwmPreProStr##_DataList,             \
        .DataLength = ListNum,                             \
        .DataCount = 0,                                    \
        .channel_num = _channel_num,                       \
        .OutArrList_p = PwmPreProStr##_ArrayList}

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

int main()
{
    C4M_DEVICE_set();

    /*=================TIMER_INIT=================*/
    //設定Timer
    // Timer 0       => ADC_Frequency_redu
    // Timer 1 A/B/C => PWM
    // Timer 2       => PWM_Frequency_redu

    TIMHWINT_LAY(TIM0INT_Str, 0, 2);
    timer0_init();

    timer1_init();

    TIMHWINT_LAY(PWM2INT_Str, 2, 1);
    timer2_init();

    /*=================PWM=================*/

    PwmPrePro_LAY(PwmPrePro_Str, 180, 3);
    //設定PWM降頻器
    uint8_t PWM_period[3];
    for (int i = 0; i < sizeof(PWM_period) / sizeof(uint8_t); i++)
        PWM_period[i] = 10;

    FREQREDU_LAY(PWMFreqRedu_Str, 3, 3, &OCR2, 1, &PWM_period);

    RT_REG_IO_LAY(PWM1ASet_Str, 0, &OCR1AL, 2, (PwmPrePro_Str.OutArrList_p + 0));
    RT_REG_IO_LAY(PWM1BSet_Str, 1, &OCR1BL, 2, (PwmPrePro_Str.OutArrList_p + 1));
    RT_REG_IO_LAY(PWM1CSet_Str, 2, &OCR1CL, 2, (PwmPrePro_Str.OutArrList_p + 2));

    uint8_t PWM_freq_TaskID[3];
    PWM_freq_TaskID[0] = FreqRedu_reg(&PWMFreqRedu_Str, &RealTimeRegPut_step, &PWM1ASet_Str, 1, 0);
    PWM_freq_TaskID[1] = FreqRedu_reg(&PWMFreqRedu_Str, &RealTimeRegPut_step, &PWM1BSet_Str, 1, 1);
    PWM_freq_TaskID[2] = FreqRedu_reg(&PWMFreqRedu_Str, &RealTimeRegPut_step, &PWM1CSet_Str, 1, 2);

    FreqRedu_en(&PWMFreqRedu_Str, PWM_freq_TaskID[0], ENABLE);
    FreqRedu_en(&PWMFreqRedu_Str, PWM_freq_TaskID[1], ENABLE);
    FreqRedu_en(&PWMFreqRedu_Str, PWM_freq_TaskID[2], ENABLE);

    /*=================ADC=================*/
    //設定ADC
    // ADC_HW_LAY();
    // hardware_set(&ADCHWSet_str);
    ADC_init();

    //設定ADC降頻器
    uint8_t ADC_period[9];
    for (int i = 0; i < sizeof(ADC_period) / sizeof(uint8_t); i++)
        ADC_period[i] = 10;

    FREQREDU_LAY(ADCFreqRedu_Str, 9, 9, &OCR0, 1, &ADC_period);

    unsigned int ADC0_read_Data;
    unsigned int ADC1_read_Data;

    ADCPOSTPRO_LAY(ADC0_PostPro_Str, 180, 0, &ADC0_read_Data);
    ADCPOSTPRO_LAY(ADC1_PostPro_Str, 180, 1, &ADC1_read_Data);

    RT_REG_IO_LAY(ADC0_result_Str, 3, &ADCL, 2, (uint8_t *)&ADC0_read_Data);
    RT_REG_IO_LAY(ADC1_result_Str, 4, &ADCL, 2, (uint8_t *)&ADC1_read_Data);

    unsigned char zero = 0;
    unsigned char one = 1;

    //切換到單通道 ADC0 and GND
    RT_FLAG_IO_LAY(ADC0_switch_channel_Str, 0, &ADMUX, 0x1f, 0, &zero);
    //觸發單通道
    RT_FLAG_IO_LAY(ADC0_trigger_convert_Str, 1, &ADCSRA, 0x40, 6, &one);

    //切換到單通道 ADC1 and GND
    RT_FLAG_IO_LAY(ADC1_switch_channel_Str, 2, &ADMUX, 0x1f, 0, &one);
    //觸發單通道
    RT_FLAG_IO_LAY(ADC1_trigger_convert_Str, 3, &ADCSRA, 0x40, 6, &one);

    uint8_t ADC_freq_TaskID[9];
    ADC_freq_TaskID[0] = FreqRedu_reg(&ADCFreqRedu_Str, &RealTimeFlagPut_step, &ADC0_switch_channel_Str, 1, 0);
    ADC_freq_TaskID[1] = FreqRedu_reg(&ADCFreqRedu_Str, &RealTimeFlagPut_step, &ADC0_trigger_convert_Str, 1, 1);
    ADC_freq_TaskID[2] = FreqRedu_reg(&ADCFreqRedu_Str, &RealTimeRegGet_step, &ADC0_result_Str, 1, 2);
    ADC_freq_TaskID[3] = FreqRedu_reg(&ADCFreqRedu_Str, &ADC0PostPro_step, &ADC0_PostPro_Str, 1, 3);
    // ADC_freq_TaskID[3] = FreqRedu_reg(&ADCFreqRedu_Str, &Pipeline_step, &SysPipeline_str, 1, 3);

    ADC_freq_TaskID[4] = FreqRedu_reg(&ADCFreqRedu_Str, &RealTimeFlagPut_step, &ADC1_switch_channel_Str, 1, 4);
    ADC_freq_TaskID[5] = FreqRedu_reg(&ADCFreqRedu_Str, &RealTimeFlagPut_step, &ADC1_trigger_convert_Str, 1, 5);
    ADC_freq_TaskID[6] = FreqRedu_reg(&ADCFreqRedu_Str, &RealTimeRegGet_step, &ADC1_result_Str, 1, 6);
    ADC_freq_TaskID[7] = FreqRedu_reg(&ADCFreqRedu_Str, &ADC1PostPro_step, &ADC1_PostPro_Str, 1, 7);

    ADC_freq_TaskID[8] = FreqRedu_reg(&ADCFreqRedu_Str, &PWMPrePro_step, &PwmPrePro_Str, 1, 8);

    // ADC_freq_TaskID[7] = FreqRedu_reg(&ADCFreqRedu_Str, &Pipeline_step, &SysPipeline_str, 1, 7);
    // ADC_freq_TaskID[8] = FreqRedu_reg(&ADCFreqRedu_Str, &Pipeline_step, &SysPipeline_str, 1, 8);

    FreqRedu_en(&ADCFreqRedu_Str, ADC_freq_TaskID[0], ENABLE);
    FreqRedu_en(&ADCFreqRedu_Str, ADC_freq_TaskID[1], ENABLE);
    FreqRedu_en(&ADCFreqRedu_Str, ADC_freq_TaskID[2], ENABLE);
    FreqRedu_en(&ADCFreqRedu_Str, ADC_freq_TaskID[3], ENABLE);
    FreqRedu_en(&ADCFreqRedu_Str, ADC_freq_TaskID[4], ENABLE);
    FreqRedu_en(&ADCFreqRedu_Str, ADC_freq_TaskID[5], ENABLE);
    FreqRedu_en(&ADCFreqRedu_Str, ADC_freq_TaskID[6], ENABLE);
    FreqRedu_en(&ADCFreqRedu_Str, ADC_freq_TaskID[7], ENABLE);
    FreqRedu_en(&ADCFreqRedu_Str, ADC_freq_TaskID[8], ENABLE);

    //設定Pipeline
    // PIPELINE_LAY(3, 5, 10);
    // PWM訊號準備
    // uint8_t pipeline_ID[3];
    // pipeline_ID[0] = Pipeline_reg(&SysPipeline_str, &ADC0PostPro_step, &ADC0_PostPro_Str, NULL);
    // pipeline_ID[1] = Pipeline_reg(&SysPipeline_str, &ADC1PostPro_step, &ADC1_PostPro_Str, NULL);
    // pipeline_ID[2] = Pipeline_reg(&SysPipeline_str, &PWMPrePro_step, &PwmPrePro_Str, NULL);

    /*=================TOTAL=================*/

    uint8_t TaskID[2];

    //將PWM降頻器登入進Timer2中斷
    TaskID[0] = HWInt_reg(&PWM2INT_Str, &FreqRedu_step, &PWMFreqRedu_Str);
    HWInt_en(&PWM2INT_Str, TaskID[0], ENABLE);

    //將ADC降頻器登入進Timer0中斷
    TaskID[1] = HWInt_reg(&TIM0INT_Str, &FreqRedu_step, &ADCFreqRedu_Str);
    HWInt_en(&TIM0INT_Str, TaskID[1], ENABLE);

    HMI_snget_matrix(HMI_TYPE_UI16, 1, PwmPrePro_Str.DataLength, PwmPrePro_Str.DataList_p);

    TRIG_NEXT_TASK(0);
    sei();

    while (1)
    {
        ;
    }

    return 0;
}

void ADC_init()
{
    //設定參考電壓:外部AVCC
    REGFPT(&ADMUX, 0xc0, REFS0, 1);

    //設定10位元轉換靠右
    REGFPT(&ADMUX, 0x20, ADLAR, 0);

    //設定非連續或觸發轉換
    REGFPT(&ADCSRA, 0x20, ADFR, DISABLE);

    //設定ADC0:2 F0:2輸入
    REGFPT(&DDRF, 0x07, 0, 0);

    //設定致能ADC
    REGFPT(&ADCSRA, 0x80, ADEN, ENABLE);

    //設定ADC時脈 clk/128
    REGFPT(&ADCSRA, 0x07, ADPS0, 7);
}

void timer0_init()
{
    REGFPT(&TCCR0, 0x48, WGM01, 1); /*CTC*/
    REGFPT(&TCCR0, 0x07, CS00, 5);  /*clk/128*/
    REGFPT(&TIMSK, 0x02, OCIE0, 1); /*致能中斷*/
    OCR0 = 107;

    //時間: 0.02 [s]
}

void timer1_init()
{
    // 設定耦合OCR1A/1B/1C
    REGFPT(&TCCR1A, 0xc0, COM1A0, 2);
    REGFPT(&TCCR1A, 0x30, COM1B0, 2);
    REGFPT(&TCCR1A, 0x0c, COM1C0, 2);

    // PWM, Phase and Frequency Correct | Top : ICRn
    REGFPT(&TCCR1A, 0x03, WGM10, 0);
    REGFPT(&TCCR1B, 0x18, WGM12, 2);

    ICR1 = 255;

    REGFPT(&TCCR1B, 0x07, CS10, 1); /*clk/1*/

    REGFPT(&DDRB, 0xe0, 5, 0x07);
}

void timer2_init()
{
    REGFPT(&TCCR2, 0x48, WGM01, 1); /*CTC*/
    REGFPT(&TCCR2, 0x07, CS00, 5);  /*clk/128*/
    REGFPT(&TIMSK, 0x80, OCIE2, 1); /*致能中斷*/
    OCR2 = 107;

    //時間: 0.02 [s]
}

void ADC0PostPro_step(void *VoidStr_p)
{
    volatile ADCPostProStr_t *Str_p = (ADCPostProStr_t *)VoidStr_p;
    Str_p->DataList_p[Str_p->DataCount] = *(Str_p->InData_p);

    if ((Str_p->DataCount + 1) == (Str_p->DataLength))
    {
        cli();
        // HMI_snput_matrix(HMI_TYPE_UI16, 1, Str_p->DataLength, Str_p->DataList_p);
        Str_p->DataCount = 0;
        sei();
    }
    else
        Str_p->DataCount++;

    TRIG_NEXT_TASK(1);
}

void ADC1PostPro_step(void *VoidStr_p)
{
    volatile ADCPostProStr_t *Str_p = (ADCPostProStr_t *)VoidStr_p;
    Str_p->DataList_p[Str_p->DataCount] = *(Str_p->InData_p);

    if ((Str_p->DataCount + 1) == (Str_p->DataLength))
    {
        cli();
        // HMI_snput_matrix(HMI_TYPE_UI16, 1, Str_p->DataLength, Str_p->DataList_p);
        Str_p->DataCount = 0;
        sei();
    }
    else
        Str_p->DataCount++;

    TRIG_NEXT_TASK(2);
}

void PWMPrePro_step(void *VoidStr_p)
{
    // printf("PWMPrePro_step\n");
    volatile PwmPreProStr_t *Str_p = (PwmPreProStr_t *)VoidStr_p;

    int DataIdx[3];

    if (Str_p->DataCount == Str_p->DataLength)
    {
        Str_p->DataCount = 0;
    }

    for (int i = 0; i < Str_p->channel_num; i++)
    {
        DataIdx[i] = Str_p->DataCount + i * Str_p->DataLength / 2;
        if (DataIdx[i] > Str_p->DataLength)
            DataIdx[i] -= Str_p->DataLength;

        Str_p->OutArrList_p[i] = Str_p->DataList_p[DataIdx[i]];
    }
    Str_p->DataCount++;

    TRIG_NEXT_TASK(0);
}
