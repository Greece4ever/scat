#include <string>
#include <iostream>
#include <array>
#define INT8 unsigned char
#define INT16 unsigned short
#include <ctype.h>
#define ACTIVATE(x) std::cout << x;
#define DEACTIVATE() std::cout << "\033[0m"
#include <fstream>
using std::string;



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
#include "./cnfg.cpp"


bool fullMatch(string& str, string target, short index) {
    if (index != 0) {
    
        if (isalpha(str[index - 1]))
            return false;
        // std::cout << "passed word start test" << "\n";
    }
    
    INT8 j = 0; // Assuming target.size() < 255
    INT16 i;
    for (i=index; i < (index + target.size()); i++) { 
        // std::cout << str[i] << " == " << target[j] << "\n";
        if (str[i] != target[j])
            return false;
        j++;
    }
    if (isalpha(str[i]) || isdigit(str[i])) { /* TODO UTF-8 CHARS */
        return false;
    }
    // std::cout << "passed word end test" << "\n";
    
    return true;
}


bool checkSymbol(string& str, string target, short index) {    
    INT8 j = 0;
    for (INT16 i=index; i < (index + target.size()); i++) { 
        if (str[i] != target[j])
            return false;
        j++;
    }
    return true;
}


bool isHexDigit(char digit) {
    int upper = toupper(digit);
    bool startA = 65 <= upper;
    bool endF =  (65 + 5) >= upper;
    return isdigit(digit) || (startA && endF);
}


bool isBinDigit(char digit) {
    return (digit == '0') || (digit == '1');
}


short handleHex(string& str, INT16 index) {
    INT16 i;
    for (i=index; i < str.size(); i++)
    {
        if ( !isHexDigit(str[i]) ) {
            break;
        }
    }
    return i;
}


short handleBin(string& str, INT16 index) {
    INT16 i;
    for (i=index; i < str.size(); i++)
    {
        if ( !isBinDigit(str[i]) ) {
            break;
        }
    }
    return i;
}


short findNumeric(string& str, INT16 index, bool& previousNumRejected) {
    if (index != 0) {
        bool isAlhpa = isalpha( tolower( str[index - 1] ) );
        if ( isalpha( tolower( str[index - 1] ) ) ) // if the previous char is alphabetic
        {
            previousNumRejected = true;
            return -1;
        }
        else if (previousNumRejected) {
            if ( isdigit(str[index - 1]) ) {
                return -1;
            }
        }
    }

    auto length = str.size();
    // Check if Hex or Binary
    if ( (length - (index + 1) ) >= 3) {
        if (str[index] == '0') {
            if (tolower(str[index + 1]) == 'x') 
                return handleHex(str, index + 2);
            else if (str[index + 1] == 'b')
                return handleBin(str, index + 2);
        }
    }

    INT16 i;
    bool seenDot = false;
    for (i=index; i < length; i++) { 
        if ( !isdigit(str[i]) ) {
            if (str[i] == '.') {
                if (!seenDot)
                    continue;
                break;
            }
            break;
        }
    }
    if ( i == index ) {
        return -1;
    }
    previousNumRejected = false;
    return i;
}



#include <string.h>
#include <iomanip>

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


int main() {

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

    // std::string b = "let a = 0xFFFFF & 1;\nlet b = 0b11111;\n";
    std::string path = "test.js";
    std::ifstream file;
    file.open(path);

    if (!file.is_open()) {
        char *err = strerror(errno);
        std::cerr << colors::RED << "[Error]" << colors::ENDC << " Cannot access " <<  colors::UNDERLINE << path << colors::ENDC << ": "<< err << "\n";
    }

    check_line(file, rep, 4, kwds, 45, symbols, 24);

    // check_character(example, rep, 4, kwds, 45, symbols, 22);
    return 0;
}

