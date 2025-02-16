//
// Created by jack on 2/13/2025.
//

#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H
#include "WindowAbstraction.h"

extern double DG_FPS;

inline void updateFPS() {
    static double realFPS = 0.0;
    static double updateTimer = 0.;

    static double lastTime = 0;
    const double currentTime = jl::windowGetTime();
    const double delta = (currentTime - lastTime);
    realFPS = 1.0 / delta;
    if (updateTimer > 0.5)
    {
        DG_FPS = realFPS;
        updateTimer = 0.0;
    } else
    {
        updateTimer += delta;
    }

    lastTime = currentTime;
}
#endif //FPSCOUNTER_H
