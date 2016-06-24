#ifndef MMA8653_H_
#define MMA8653_H_

class MMA8653 {
public:
    void setup();
    bool getXYZ(float& X, float& Y, float& Z);
};

#endif
