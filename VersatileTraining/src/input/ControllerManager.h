#pragma once
#include <pch.h>

struct ButtonState {
    bool isPressed;
    std::chrono::steady_clock::time_point lastUpdateTime;
};

struct Input {
    int index;
    bool pressed;
    std::string name;
};
class ControllerManager {
private:

    
    LPDIRECTINPUTDEVICE8 joystick;
   
    using ButtonCallback = std::function<void()>;

    std::unordered_map<int, ButtonCallback> buttonCallbacks;
    std::unordered_map<int, ButtonState> buttonStates;
    bool R1AlreadyPressed = false;
    std::chrono::steady_clock::time_point lastUpdateTime;
    std::map<std::string, struct Input> m_inputMap;

    
    

public:
    LPDIRECTINPUT8 dinput = nullptr;
    std::vector<LPDIRECTINPUTDEVICE8> controllers;
    void cleanup();
    void registerButtonCallback(int buttonIndex, ButtonCallback callback);
    void checkForButtonPress(int buttonIndex);
    void enumerateControllers();
    void rollLeft();
    void rollRight();
     BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* instance, VOID* context);

     void initializeCallBacks();
};