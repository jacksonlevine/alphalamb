//
// Created by jack on 2/16/2025.
//

#ifndef WINDOWABSTRACTION_H
#define WINDOWABSTRACTION_H


namespace jl
{
    struct Window;

    double windowGetTime();
    void windowUncaptureMouse(jl::Window* window);
    void windowCaptureMouse(jl::Window* window);
    void setWindowShouldClose(jl::Window* window, bool shouldClose);
    void* getWindowUserPointer(jl::Window* window);
    void setWindowUserPointer(jl::Window* window, void* userPointer);

    enum MouseButton
    {
        LEFT_BUTTON,
        RIGHT_BUTTON,
        MIDDLE_BUTTON,
    };

    enum KeyboardKey
    {
        KBACKTICK,
        K1,
        K2,
        K3,
        K4,
        K5,
        K6,
        K7,
        K8,
        K9,
        K0,
        K_,
        KEQUAL,
        KBACKSPACE,
        KTAB,
        
    };

}




#endif //WINDOWABSTRACTION_H
