/* Included in main.cpp */

#ifndef INT16
    #define INT16 unsigned short
    #define INT8 unsigned char
    #include <ctype.h>
    #include <string>
    using std::string;
#endif

// _ ()

bool fullMatch(string& str, string target, short index) {
    if (index != 0) {
    
        if (isalpha(str[index - 1]) || str[index - 1] == '_' || str[index - 1] == '.' )
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
    if (isalpha(str[i]) || isdigit(str[i]) || str[i] == '_' ) { /* TODO UTF-8 CHARS */
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
