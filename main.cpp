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
#define WARNING(x) std::cerr << colors::YELLOW << "[Warning] " << colors::ENDC << x << "\n"
// #include "./cnfg.cpp"

#include "./parsing/number.hpp"
#include "./parsing/general.hpp"

#include <string.h>
#define ___MAIN___ 0xFFFF
#include "./db/SQL.cpp"

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
    std::ifstream&     file_handler, 
    vector<std::array<string, 3>> rep,   // INT8 rep_size, // Quotes, comments
    vector<std::array<string, 2>>      kwds,  // INT8 kwd_size, // Keywords
    string smbls[][2], INT8 smbl_size // Symbols may not be full matches like keywrods
) 
{
    INT8 kwd_size = kwds.size();
    INT8 rep_size = rep.size();


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
            
            // Symbols (+, -)
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

#include <filesystem>
#include <pwd.h>
#include <unistd.h>

string get_home_dir() {
    char* home;
    if ( (home = getenv("HOME")) == NULL) {
        home = getpwuid(getuid())->pw_dir;
    }
    return home;
}


int main(int argc, char *argv[]) {
    string symbols[][2] = {
        {"-", colors::YELLOW},   {"*", colors::YELLOW},   {"/", colors::YELLOW},
        {"&",  colors::YELLOW},  {"|",  colors::YELLOW},  {"^",  colors::YELLOW},

        {"%", colors::YELLOW},   {"~", colors::YELLOW},  {".",  colors::YELLOW},  
        {"(",  colors::BOLD},   {")",  colors::BOLD},  {"=",  colors::YELLOW},
        
        {"[",  colors::WHITE},   {"]",  colors::WHITE}, {":",  colors::YELLOW},
        {"{",  colors::WHITE},   {"}",  colors::WHITE},
        {"<",  colors::YELLOW},  {">",  colors::YELLOW}, 

        {";",  colors::WHITE},   {"++", colors::YELLOW},  {"!",  colors::YELLOW},            
    };

    const int SYMBOL_SIZE = sizeof(symbols) / sizeof(symbols[0]);    

    const string storage_path = get_home_dir() + "/.config/scat/";
    DB database(storage_path + "sources.sqlite3");


    if (!database.isOpen()) {
        auto dir = std::filesystem::create_directories(storage_path);
        database.connect(database.getPath());
        if (!database.isOpen()) {
            ERROR("Could not load sqlite3 db on path \"" + storage_path + "\"");
            exit(EXIT_FAILURE);
        }
    }


    switch (argc)
    {
        case 0:
        case 1: 
        {
            ERROR("No file or arguments specified");
            exit(EXIT_FAILURE);
            break;
        }
        case 2:
        {
            string path = (std::string)argv[1];
            string file_ext = findExtension(path);
            get_kwds(database, file_ext);

            std::ifstream file;
            file.open(path);

            if (!file.is_open()) {
                char *err = strerror(errno);
                ERROR("Cannot access " + (std::string)"\"" + colors::UNDERLINE + path + colors::ENDC + "\"" + ": " + err);
                exit(EXIT_FAILURE);
            }
            // Here it is 
            check_line(file, cs::REP_KWDS, cs::KEYWORDS, symbols, SYMBOL_SIZE);
            break;
        }
        case 3:
        {   
            std::string option = (std::string)argv[1];
            std::string path = (std::string)argv[2];


            if (option == "--parse") {
                langData data = parse_file(path);
                printKwds(data);
            }
            else if (option == "--default") {
                langData data = parse_file(path);
                data.file_extension = ".default";
                KWD_ID id_data = createLang(database, data);
                createLangKwdConenctions(database, id_data);
            }
            else if (option == "--delete") {
                deleteLang(database, path);
            }   
            break;
        }
        case 4:
        {
            string option0 = (string)argv[1];
            string option1 = (string)argv[2];
            string path =    (string)argv[3];
            if (option0 != "--parse")
            {
                ERROR("Unrecognised 1st argument \"" + option0 + "\"" + ", must be \"--parse\"");
                exit(EXIT_FAILURE);
            }
            if (option1 != "--save")
            {
                ERROR("Unrecognised 2nd argument \"" + option1 + "\", must be \"" + "--save\"");
                exit(EXIT_FAILURE);
            }
            langData data = parse_file(path);
            printKwds(data);
            string *a = new string;
            std::cout << "Save into SQLite3 database, Y or N: ";
            std::cin >> *a;
            if (*a != "Y")
                exit(EXIT_FAILURE);
            delete a;
            KWD_ID id_data = createLang(database, data);
            createLangKwdConenctions(database, id_data);
            break;
        }
        default:
            ERROR("Too many arguments (" + std::to_string(argc) + ")");
            exit(EXIT_FAILURE);
            break;
    }
    return EXIT_SUCCESS;
}
