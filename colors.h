#ifndef __BASE_CAT__
    #include <string>
    using std::string;
#endif
#ifndef _GLIBCXX_MAP
    #include <map>
#endif

namespace colors {
    string PURPLE =    "\033[95m";
    string BLUE =      "\033[94m";
    string OKCYAN =    "\033[96m";
    string GREEN =     "\033[92m";
    string DARKGREEN = "\033[32m";
    string YELLOW =    "\033[93m";
    string RED =       "\033[91m";
    string ENDC =      "\033[0m";
    string BOLD =      "\033[1m";
    string UNDERLINE = "\033[4m";
    string WHITE =     "\033[97m";
    string GREY =      "\033[30m";
    string DEFAULT = colors::OKCYAN;

    std::map<int, std::string> IScodes = {
            {1,  colors::PURPLE },
            {2,  colors::BLUE },
            {3,  colors::OKCYAN },
            {4,  colors::GREEN },
            {5,  colors::DARKGREEN },
            {6,  colors::YELLOW },
            {7,  colors::RED },
            {8,  colors::ENDC },
            {9,  colors::BOLD },
            {10, colors::UNDERLINE },
            {11, colors::WHITE },
            {12, colors::GREY },
    };

    std::map<std::string, int> SIcodes = {
        { "PURPLE",    1},
        { "BLUE",      2},
        { "OKCYAN",    3},
        { "GREEN",     4},
        { "DARKGREEN", 5},
        { "YELLOW",    6},
        { "RED",       7},
        { "ENDC",      8},
        { "BOLD",      9},
        { "UNDERLINE", 10},
        { "WHITE",     11},
        { "GREY",      12}
    };
};
