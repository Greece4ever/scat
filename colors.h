#ifndef __BASE_CAT__
    #include <string>
    using std::string;
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
};
