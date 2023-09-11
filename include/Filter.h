#ifndef FILTER_H_
#define FILTER_H_

/* Filter Definition */
#define GAINBtw50hz (3.450423889e+02f)  // 50Hz Butterworth LPF

/***** Private TypeDef *****/
typedef struct {
    float xv[4];
    float yv[4];
    float input;
    float output;
} Bw50HzLPFTypeDef;  // 50Hz Butterworth LPF
extern Bw50HzLPFTypeDef AltitudeLPF_50;

/***** Private Function *****/
float Butterworth50HzLPF(Bw50HzLPFTypeDef* pLPF);  // 50Hz Butterworth LPF

#endif /* FILTER_H_ */
