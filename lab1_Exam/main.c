#include "c4mlib.h"
// #define length 255
#define length 45
#define setA_INIT 0.5
#define setB_INIT 0.45
#define setC_INIT 0.35
#define setD_INIT 0.25
#define setx0_INIT 0.15

#define set_INIT                                               \
    {                                                          \
        setA_INIT, setB_INIT, setC_INIT, setD_INIT, setx0_INIT \
    }
struct init
{
    float A;
    float B;
    float C;
    float D;
    float x0;
} set = set_INIT;

void iteration(int k);
float x[length + 1], u[length], y[length];
int main()
{
    C4M_DEVICE_set();

    printf("Enter A , B , C , D , x0\n");
    HMI_snput_struct("f32_5", sizeof(set), &set);
    HMI_snget_struct("f32_5", sizeof(set), &set);
    printf("A=%3.2f B=%3.2f C=%3.2f D=%3.2f x0=%3.2f\n", set.A, set.B, set.C, set.D, set.x0);
    x[0] = set.x0;

    printf("Enter u(k)\n");
    HMI_snget_matrix(8, 1, length, &u); //8 f32
    iteration(0);
    printf("Output y(k)\n");
    HMI_snput_matrix(8, 1, length, &y);
    printf("End\n");
    return 0;
}

void iteration(int k)
{
    if (k != length)
    {
        x[k + 1] = set.A * x[k] + set.B * u[k];
        y[k] = set.C * x[k] + set.D * u[k];
        iteration(k + 1);
    }
}