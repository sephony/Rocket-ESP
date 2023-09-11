#include "Filter.h"

Bw50HzLPFTypeDef AltitudeLPF_50;

float Butterworth50HzLPF(Bw50HzLPFTypeDef* pLPF) {
    pLPF->xv[0] = pLPF->xv[1];
    pLPF->xv[1] = pLPF->xv[2];
    pLPF->xv[2] = pLPF->xv[3];
    pLPF->xv[3] = pLPF->input / GAINBtw50hz;
    pLPF->yv[0] = pLPF->yv[1];
    pLPF->yv[1] = pLPF->yv[2];
    pLPF->yv[2] = pLPF->yv[3];
    pLPF->yv[3] = (pLPF->xv[0] + pLPF->xv[3]) + 3 * (pLPF->xv[1] + pLPF->xv[2]) +
                  (0.5320753683f * pLPF->yv[0]) + (-1.9293556691f * pLPF->yv[1]) +
                  (2.3740947437f * pLPF->yv[2]);

    pLPF->output = pLPF->yv[3];
    return pLPF->output;
}
