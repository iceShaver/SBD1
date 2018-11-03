//
// Created by kamil on 03.11.18.
//

#ifndef SBD_1_TERMINAL_HH
#define SBD_1_TERMINAL_HH

#include <iostream>

namespace Terminal {
    enum class Color{
        FG_RED = 31,
        FG_GREEN = 32,
        FG_BLUE = 34,
        FG_DEFAULT = 39,
        BG_RED = 41,
        BG_GREEN = 42,
        BG_BLUE = 44,
        BG_DEFAULT = 49
    };
    void set_color(Color color){
        std::cout << "\033[" << static_cast<int>(color) << 'm';
    }
}
#endif //SBD_1_TERMINAL_HH
