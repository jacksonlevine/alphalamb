//
// Created by jack on 2/13/2025.
//

#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H

extern double DG_FPS;

inline void updateFPS() {
    static double realFPS = 0.0;
    static float updateTimer = 0.0f;

    static double lastTime = 0;
    double currentTime = glfwGetTime();
    double delta = (currentTime - lastTime);
    realFPS = 1.0 / delta;
    if (updateTimer > 0.5f)
    {
        DG_FPS = realFPS;
        updateTimer = 0.0f;
    } else
    {
        updateTimer += delta;
    }

    lastTime = currentTime;
}
#endif //FPSCOUNTER_H
