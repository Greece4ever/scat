#include <ctype.h>
#ifndef INT16
    #define INT16 unsigned short
    #include <string>
    using std::string;
#endif

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
