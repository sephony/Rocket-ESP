#ifndef ROCKET_H_
#define ROCKET_H_

#include <Arduino.h>

class Rocket {
public:
    Rocket() = default;
    ~Rocket();
    void updateMS5611();

private:
    float H0 = 0.0;
    double T_detach;
    double T_para;
    double H_para;
    double T_protectPara;
};

#endif /* ROCKET_H_ */
