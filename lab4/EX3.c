/*
 * waveGenerator
 * frequency : 10Hz
 * amplitude : 1V
 * offset : 0.5V
*/

#include "c4mlib.h"

#define N 2

void ADC_single_PostPro_step(void *VoidStr_p);
void ADC_diff_PostPro_step(void *VoidStr_p);

void ADC_init();
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

    //設定ADC interrupt
    ADCHWINT_LAY(ADCHWINT_Str, 0, 3);
    hardware_set(&ADCHWINT_Str);

    //設定降頻器配置
    int period[4];
    for (int i = 0; i < sizeof(period) / sizeof(int); i++)
        period[i] = 33;

    FREQREDU_LAY(FreqRedu_Str, 2 * N, 2 * N, &OCR3A, 2, &period);

    unsigned int ADC_read_single_Data;
    unsigned int ADC_read_diff_Data;

    RT_REG_IO_LAY(ADC_single_result_Str, 0, &ADCL, 2, (uint8_t *)&ADC_read_single_Data);
    RT_REG_IO_LAY(ADC_diff_result_Str, 1, &ADCL, 2, (uint8_t *)&ADC_read_diff_Data);

    ADCPOSTPRO_LAY(ADC_single_PostPro_Str, 200, 0, &ADC_read_single_Data);
    ADCPOSTPRO_LAY(ADC_diff_PostPro_Str, 200, 1, &ADC_read_diff_Data);

    unsigned char one = 1;
    unsigned char two = 2;
    unsigned char eighteen = 18;

    //切換到單通道 ADC2  and GND
    RT_FLAG_IO_LAY(ADC_single_channel_Str, 0, &ADMUX, 0x1f, 0, &one);
    //觸發單通道
    RT_FLAG_IO_LAY(ADC0_trigger_convert_Str, 1, &ADCSRA, 0x40, 6, &one);
    //切換到雙通道 ADC2 -> V+, ADC1 -> V-
    RT_FLAG_IO_LAY(ADC_diff_channel_Str, 2, &ADMUX, 0x1f, 0, &eighteen);
    //觸發雙通道
    RT_FLAG_IO_LAY(ADC12_trigger_convert_Str, 3, &ADCSRA, 0x40, 6, &one);

    uint8_t freq_TaskID[4];
    freq_TaskID[0] = FreqRedu_reg(&FreqRedu_Str, &RealTimeFlagPut_step, &ADC_single_channel_Str, 1, 0);
    freq_TaskID[1] = FreqRedu_reg(&FreqRedu_Str, &RealTimeFlagPut_step, &ADC0_trigger_convert_Str, 1, 1);
    freq_TaskID[2] = FreqRedu_reg(&FreqRedu_Str, &RealTimeFlagPut_step, &ADC_diff_channel_Str, 1, 2);
    freq_TaskID[3] = FreqRedu_reg(&FreqRedu_Str, &RealTimeFlagPut_step, &ADC12_trigger_convert_Str, 1, 3);

    FreqRedu_en(&FreqRedu_Str, freq_TaskID[0], ENABLE);
    FreqRedu_en(&FreqRedu_Str, freq_TaskID[1], ENABLE);
    FreqRedu_en(&FreqRedu_Str, freq_TaskID[2], ENABLE);
    FreqRedu_en(&FreqRedu_Str, freq_TaskID[3], ENABLE);

    //設定Pipeline
    PIPELINE_LAY(2, 4, 10);

    //單/雙通道資料後處理
    uint8_t pipeline_TaskID[2];
    pipeline_TaskID[0] = Pipeline_reg(&SysPipeline_str, &ADC_single_PostPro_step, &ADC_single_PostPro_Str, NULL);
    pipeline_TaskID[1] = Pipeline_reg(&SysPipeline_str, &ADC_diff_PostPro_step, &ADC_diff_PostPro_Str, NULL);

    uint8_t TaskID[4];

    //將降頻器登入進Timer中斷
    TaskID[0] = HWInt_reg(&TIMINT_Str, &FreqRedu_step, &FreqRedu_Str);
    HWInt_en(&TIMINT_Str, TaskID[0], ENABLE);

    //將ADC_single 結果登入進ADC interrupt
    TaskID[1] = HWInt_reg(&ADCHWINT_Str, &RealTimeRegGet_step, &ADC_single_result_Str);
    HWInt_en(&ADCHWINT_Str, TaskID[1], ENABLE);

    //將ADC_diff   結果登入進ADC interrupt
    TaskID[2] = HWInt_reg(&ADCHWINT_Str, &RealTimeRegGet_step, &ADC_diff_result_Str);
    HWInt_en(&ADCHWINT_Str, TaskID[2], ENABLE);

    //將pipeline登入進ADC interrupt
    TaskID[3] = HWInt_reg(&ADCHWINT_Str, &Pipeline_step, &SysPipeline_str);
    HWInt_en(&ADCHWINT_Str, TaskID[3], ENABLE);

    sei();
    TRIG_NEXT_TASK(0);

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

    //設定ADC1:2 F1:2c皆輸入
    REGFPT(&DDRF, 0x06, 0, 0);

    //設定致能ADC
    REGFPT(&ADCSRA, 0x80, 7, ENABLE);

    //設定致能ADC Interrupt
    REGFPT(&ADCSRA, 0x08, 3, ENABLE);

    //設定ADC時脈 clk/128
    REGFPT(&ADCSRA, 0x07, 0, 7);
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

void ADC_single_PostPro_step(void *VoidStr_p)
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

    TRIG_NEXT_TASK(1);
}

void ADC_diff_PostPro_step(void *VoidStr_p)
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