//
// Created by kamil on 03.11.18.
//

#ifndef SBD_1_TOOLS_HH
#define SBD_1_TOOLS_HH

#include <functional>
#include "config.hh"

template<typename _F>
void verbose(_F f) {
    if (Config::verbose)
        std::invoke(f);
}

template<typename _F>
void debug(_F f) {
    if (Config::debug)
        std::invoke(f);
}

#endif //SBD_1_TOOLS_HH
