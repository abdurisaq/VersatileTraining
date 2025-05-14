#include "pch.h"
#include "src/core/VersatileTraining.h"


int VersatileTraining::getVirtualKeyCode(const std::string& keyName) {
    if (keyName.length() == 1 && keyName[0] >= 'A' && keyName[0] <= 'Z') {
        return keyName[0]; 
    }
    else if (keyName.length() == 1 && keyName[0] >= '0' && keyName[0] <= '9') {
        return keyName[0]; 
    }

    static const std::unordered_map<std::string, int> keyMap = {
        
        {"F1", VK_F1}, {"F2", VK_F2}, {"F3", VK_F3}, {"F4", VK_F4},
        {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7}, {"F8", VK_F8},
        {"F9", VK_F9}, {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},

    
        {"LeftShift", VK_LSHIFT}, {"RightShift", VK_RSHIFT},
        {"LeftControl", VK_LCONTROL}, {"RightControl", VK_RCONTROL},
        {"LeftAlt", VK_LMENU}, {"RightAlt", VK_RMENU},

        {"SpaceBar", VK_SPACE}, {"Tab", VK_TAB}, {"CapsLock", VK_CAPITAL},
        {"Enter", VK_RETURN}, {"Escape", VK_ESCAPE}, {"BackSpace", VK_BACK},

        {"Insert", VK_INSERT}, {"Delete", VK_DELETE}, {"Home", VK_HOME},
        {"End", VK_END}, {"PageUp", VK_PRIOR}, {"PageDown", VK_NEXT},
        {"Up", VK_UP}, {"Down", VK_DOWN}, {"Left", VK_LEFT}, {"Right", VK_RIGHT},

        {"ScrollLock", VK_SCROLL}, {"Pause", VK_PAUSE},
        {"NumLock", VK_NUMLOCK}, {"Tilde", VK_OEM_3}, {"Underscore", VK_OEM_MINUS},
        {"Add", VK_OEM_PLUS}, {"LeftBracket", VK_OEM_4}, {"RightBracket", VK_OEM_6},
        {"BackSlash", VK_OEM_5}, {"Semicolon", VK_OEM_1}, {"Quote", VK_OEM_7},
        {"Comma", VK_OEM_COMMA}, {"Period", VK_OEM_PERIOD}, {"Slash", VK_OEM_2},

        {"NumPadZero", VK_NUMPAD0}, {"NumPadOne", VK_NUMPAD1}, {"NumPadTwo", VK_NUMPAD2},
        {"NumPad3", VK_NUMPAD3}, {"NumPadFour", VK_NUMPAD4}, {"NumPadFive", VK_NUMPAD5},
        {"NumPadSix", VK_NUMPAD6}, {"NumPadSeven", VK_NUMPAD7}, {"NumPadEight", VK_NUMPAD8},
        {"NumPadNine", VK_NUMPAD9}, {"Multiply", VK_MULTIPLY}, {"Add", VK_ADD},
        {"Subtract", VK_SUBTRACT}, {"Decimal", VK_DECIMAL}, {"Divide", VK_DIVIDE}
    };

    auto it = keyMap.find(keyName);
    if (it != keyMap.end()) {
        return it->second;
    }

    return 0; 
}

std::string VersatileTraining::getKeyName(int virtualKeyCode) {
    if (virtualKeyCode >= 'A' && virtualKeyCode <= 'Z') {
        
        return std::string(1, static_cast<char>(virtualKeyCode));
    }
    else if (virtualKeyCode >= '0' && virtualKeyCode <= '9') {
        
        return std::string(1, static_cast<char>(virtualKeyCode));
    }

    
    static const std::unordered_map<int, std::string> keyNameMap = {
        // fn keys
        {VK_F1, "F1"}, {VK_F2, "F2"}, {VK_F3, "F3"}, {VK_F4, "F4"},
        {VK_F5, "F5"}, {VK_F6, "F6"}, {VK_F7, "F7"}, {VK_F8, "F8"},
        {VK_F9, "F9"}, {VK_F10, "F10"}, {VK_F11, "F11"}, {VK_F12, "F12"},

        // Mod keys
        {VK_SHIFT, "LeftShift"}, {VK_LSHIFT, "LeftShift"}, {VK_RSHIFT, "RightShift"},
        {VK_CONTROL, "LeftControl"}, {VK_LCONTROL, "LeftControl"}, {VK_RCONTROL, "RightControl"},
        {VK_MENU, "LeftAlt"}, {VK_LMENU, "LeftAlt"}, {VK_RMENU, "RightAlt"},

        
        {VK_SPACE, "SpaceBar"}, {VK_TAB, "Tab"}, {VK_CAPITAL, "CapsLock"},
        {VK_RETURN, "Enter"}, {VK_ESCAPE, "Escape"}, {VK_BACK, "BackSpace"},

        {VK_INSERT, "Insert"}, {VK_DELETE, "Delete"}, {VK_HOME, "Home"},
        {VK_END, "End"}, {VK_PRIOR, "PageUp"}, {VK_NEXT, "PageDown"},
        {VK_UP, "Up"}, {VK_DOWN, "Down"}, {VK_LEFT, "Left"}, {VK_RIGHT, "Right"},

        
        {VK_SCROLL, "ScrollLock"}, {VK_PAUSE, "Pause"},
        {VK_NUMLOCK, "NumLock"}, {VK_OEM_3, "Tilde"}, {VK_OEM_MINUS, "Underscore"},
        {VK_OEM_PLUS, "Add"}, {VK_OEM_4, "LeftBracket"}, {VK_OEM_6, "RightBracket"},
        {VK_OEM_5, "BackSlash"}, {VK_OEM_1, "Semicolon"}, {VK_OEM_7, "Quote"},
        {VK_OEM_COMMA, "Comma"}, {VK_OEM_PERIOD, "Period"}, {VK_OEM_2, "Slash"},

        // Numpad
        {VK_NUMPAD0, "NumPadZero"}, {VK_NUMPAD1, "NumPadOne"}, {VK_NUMPAD2, "NumPadTwo"},
        {VK_NUMPAD3, "NumPad3"}, {VK_NUMPAD4, "NumPadFour"}, {VK_NUMPAD5, "NumPadFive"},
        {VK_NUMPAD6, "NumPadSix"}, {VK_NUMPAD7, "NumPadSeven"}, {VK_NUMPAD8, "NumPadEight"},
        {VK_NUMPAD9, "NumPadNine"}, {VK_MULTIPLY, "Multiply"}, {VK_ADD, "Add"},
        {VK_SUBTRACT, "Subtract"}, {VK_DECIMAL, "Decimal"}, {VK_DIVIDE, "Divide"},

    
    };

    auto it = keyNameMap.find(virtualKeyCode);
    if (it != keyNameMap.end()) {
        return it->second;
    }

    return "UNKNOWN"; 
}