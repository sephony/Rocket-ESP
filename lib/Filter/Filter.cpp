#include "Filter.h"

Bw50HzLPFTypeDef AltitudeLPF_50;
/*
这段代码实现的是一个50Hz的Butterworth低通滤波器。Butterworth滤波器是一种在通带内具有平坦频率响应的滤波器，通常用于消除高频噪声。

在这段代码中，`Butterworth50HzLPF`函数接收一个`Bw50HzLPFTypeDef`类型的指针`pLPF`，该类型定义了滤波器的状态和输入输出值。

滤波器的工作原理如下：

1. 首先，滤波器的输入值`pLPF->input`被除以一个增益值`GAINBtw50hz`，然后存储在`pLPF->xv[3]`中。同时，`pLPF->xv`数组中的其他元素被向前移动一位，以便为新的输入值腾出空间。

2. 然后，滤波器的输出值`pLPF->yv[3]`被计算为`pLPF->xv`和`pLPF->yv`数组中的元素的加权和。这个加权和的权重是固定的，由Butterworth滤波器的设计决定。

3. 最后，计算出的输出值`pLPF->yv[3]`被存储在`pLPF->output`中，并作为函数的返回值。

在这个过程中，高频噪声被滤除，因为它们在加权和中的权重较小，而低频信号被保留，因为它们在加权和中的权重较大。

在你的代码中，`AltitudeLPF_50.input = height;`这行代码将高度值作为滤波器的输入，然后`height_filter = Butterworth50HzLPF(&AltitudeLPF_50);`这行代码调用滤波器函数，得到滤波后的高度值。这样，高度值中的高频噪声就被消除了。
*/
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
