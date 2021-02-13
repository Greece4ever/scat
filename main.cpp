#define __BASE_CAT__
#include <string>
#include <iostream>
#include <ctype.h>
using std::string;
#include "colors.h"
#define INT8 unsigned char
#define INT16 unsigned short
#define ACTIVATE(x) std::cout << x;
#define DEACTIVATE() std::cout << "\033[0m"
#include <fstream>

#define ERROR(x) std::cerr << colors::RED << "[Error] " << colors::ENDC << x << "\n"
#include "./cnfg.cpp"

#include "./parsing/number.hpp"
#include "./parsing/general.hpp"

#include <string.h>

// a2 216
// Α2 ΣΕΛΙΔΑ 216
// b1 b2, 255
// 221 ekthesi

void printDelimiter(std::string& delimeter, INT16& index) {
    std::cout << delimeter[0];
    for (INT8 k=1; k < delimeter.size(); k++) 
    {
        std::cout << delimeter[k];
        index++;
    }
}


void awaitQuote(bool& seenQuote, std::string& line, std::string& endDelimeter, INT16& index) {
    /* Function-handler for when waiting for a missing quote (") */
    if (seenQuote) {
        if (checkSymbol(line, endDelimeter, index))  // if End delimeter is found
        {
            printDelimiter(endDelimeter, index);
            DEACTIVATE();
            seenQuote = false;
        }
        else { // end delimeter not found
            std::cout << line[index];
        }
    }
}

void handleDigit(std::string& line, INT16& i, bool& previousNumRejected) {
    short index = findNumeric(line, i, previousNumRejected);
    if (index != -1) {
        ACTIVATE(colors::DARKGREEN);
        for (INT16 k=i; k < index; k++) {
            std::cout << line[k];
            i++;
        }
        DEACTIVATE();
    }
}

void check_line(
    std::ifstream&    file_handler, 
    string rep[][3],  INT8 rep_size, // Quotes, comments
    string kwds[][2], INT8 kwd_size, // Keywords
    string smbls[][2], INT8 smbl_size // Symbols may not be full matches like keywrods
) 
{
    string line;
        bool seenQuote = false;
        bool previousNumRejected = false;
    string endDelimiter;
    while(getline(file_handler, line)) 
    {
        line.push_back('\n');
        for (INT16 i=0; i < line.size(); i++) // each letter
        {
            if (seenQuote) {
                awaitQuote(seenQuote, line,endDelimiter, i);
                continue;
            }

            // Check for quotes, comments, etc
            for (INT8 j=0; j < rep_size; j++)
            {
                if (checkSymbol(line, rep[j][0], i)) 
                {
                    seenQuote = true;
                    endDelimiter = rep[j][1]; // End of quote (eg /* end is */)
                    ACTIVATE(rep[j][2]);
                    printDelimiter(rep[j][0], i);  
                    goto end;                  
                }
            } 

            // Check for all keywords
            for (INT8 j=0; j < kwd_size; j++)
            {
                if (fullMatch(line, kwds[j][0], i)) {
                    ACTIVATE(kwds[j][1]);
                    printDelimiter(kwds[j][0], i);
                    DEACTIVATE();
                    goto end;
                }
            }

            for (INT8 j=0; j < smbl_size; j++)
            {
                if (checkSymbol(line, smbls[j][0], i)) {
                    ACTIVATE(smbls[j][1]);
                    printDelimiter(smbls[j][0], i);
                    DEACTIVATE(); // Deactivate color
                    goto end;
                }
            }


            if ( isdigit(line[i]) )
                handleDigit(line, i, previousNumRejected);

            std::cout << line[i];
        end:
            continue; 
        }
    }
}


int main(int argc, char *argv[]) {
    string rep[][3] = {
        {"'",  "'",  colors::GREEN}, // Quote '
        {"\"", "\"", colors::GREEN}, // Quote ""
        {"/*", "*/", colors::GREY},   // Comment
        {"//", "\n", colors::GREY}
    };
    
    string symbols[][2] = {
        {".", colors::YELLOW},
        {"(", colors::WHITE},
        {")", colors::WHITE},
        {"[", colors::WHITE},
        {"]", colors::WHITE},
        {"}", colors::WHITE},
        {"{", colors::WHITE},
        {"<", colors::YELLOW},
        {">", colors::YELLOW},
        {";", colors::WHITE},
        {"++", colors::YELLOW},
        {"!", colors::YELLOW},

        {"+=", colors::YELLOW},
        {"-=", colors::YELLOW},
        {"/=", colors::YELLOW},
        {"*=", colors::YELLOW},

        {"=", colors::YELLOW},
        {"-", colors::WHITE},
        {"+", colors::WHITE},
        {"/", colors::WHITE},
        {"*", colors::WHITE},
        {"&", colors::YELLOW},
        {"|", colors::YELLOW},
        {"^", colors::YELLOW},

    };


    if (argc <= 1) {
        ERROR("No file specified");
        exit(EXIT_FAILURE);
    }
    
    std::string path = (std::string)argv[1];
    std::ifstream file;
    file.open(path);

    if (!file.is_open()) {
        char *err = strerror(errno);
        ERROR("Cannot access " + (std::string)"\"" + colors::UNDERLINE + path + colors::ENDC + "\"" + ": " + err);
    }

    check_line(file, rep, 4, kwds, 45, symbols, 24);
    return 0;
}
