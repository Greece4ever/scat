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
int a = 0;

#define f(x) std::cout << "DEBUG " << __LINE__ << "\n";

#include "./db/SQL.cpp"

typedef unsigned int uint;

using std::array;
using std::tuple;


void printDelimiter(std::string& delimeter, INT16& index) {
    std::cout << delimeter[0];
    for (INT8 k=1; k < delimeter.size(); k++) 
    {
        std::cout << delimeter[k];
        index++;
    }
}


void awaitQuote(bool& seenQuote, std::string& line, std::vector<string>* endDelimeter, INT16& index, bool& include_str) {
    /* Function-handler for when waiting for a missing quote (") */

    if (seenQuote) {
        for (uint i=0; i < endDelimeter->size(); i++)
        {   
            if (checkSymbol(line, endDelimeter->at(i), index))  // if End delimeter is found
            {
                if (!include_str)
                    DEACTIVATE();
                printDelimiter(endDelimeter->at(i), index);
                DEACTIVATE();
                seenQuote = false;
                include_str = true;
                return;
            }
        }
        std::cout << line[index];
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

// { "def", colors::YELLOW, "(", colors::YELLOW }

template<typename string_stream>
void check_line(
    string_stream& file_handler, 
    vector<std::array<string, 2>> kwds={},     // INT8 kwd_size, // Keywords
    vector< tuple<string, vector<string>, string> > rep={},      // Quotes, comments (need an end delimeter)
    vector< 
        tuple<array<string, 2>,// (START, END) color
             string,           // START KEYWORD
             vector<string>>   // END KEYWORDS
    > kwds_cont={},
    vector<array<string, 2>> smbls={} // Symbols may not be full matches like keywrods
) 
{
    INT8 kwd_size = kwds.size();
    INT8 rep_size = rep.size();

    string line;
        bool seenQuote = false;
        bool previousNumRejected = false;

    std::vector<string>* endDelimiter = nullptr;

    unsigned int line_count = 0;
    bool include_str = true;

    while(getline(file_handler, line)) 
    {
        line_count++;
        line.push_back('\n');

        for (INT16 i=0; i < line.size(); i++) // each letter
        {
            if (seenQuote) {
                awaitQuote(seenQuote, line, endDelimiter, i, include_str);
                continue;
            }

            // Check for quotes, comments, etc
            for (INT8 j=0; j < rep_size; j++)
            {
                std::string& keyword = std::get<0>(rep[j]);

                if (checkSymbol(line, keyword, i)) 
                {
                    seenQuote    = true;
                    endDelimiter = &std::get<1>(rep[j]); // End of quote (eg /* end is */)
                    ACTIVATE( std::get<2>(rep[j])  );
                        printDelimiter(keyword, i);  
                    goto end;                  
                }
            } 

            

            for (INT8 j=0; j < kwds_cont.size(); j++) {
                std::string& keyword        = std::get<1>(kwds_cont[j]);
                std::string& kwd_color      = std::get<0>(kwds_cont[j])[0];
                std::string& next_color     = std::get<0>(kwds_cont[j])[1];


                if (fullMatch(line, keyword, i)) {
                    ACTIVATE( kwd_color );
                        printDelimiter(keyword, i);
                    DEACTIVATE();
                    ACTIVATE( next_color );
                        include_str = false;
                        seenQuote = true;
                        endDelimiter = &(std::get<2>(kwds_cont[j]));
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
            for (INT8 j=0; j < smbls.size(); j++)
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
    vector<array<string, 2>> symbols = {
        {"-", colors::YELLOW},   {"*", colors::YELLOW},   {"/", colors::YELLOW},
        {"&",  colors::YELLOW},  {"|",  colors::YELLOW},  {"^",  colors::YELLOW},

        {"%", colors::YELLOW},   {"~", colors::YELLOW},   {".",  colors::YELLOW},  
        {"(",  colors::BOLD},    {")",  colors::BOLD},    {"=",  colors::YELLOW},
        
        {"[",  colors::WHITE},   {"]",  colors::WHITE}, {":",  colors::YELLOW},
        {"{",  colors::WHITE},   {"}",  colors::WHITE}, 
        {"<",  colors::YELLOW},  {">",  colors::YELLOW}, 

        {";",  colors::WHITE},   {"+", colors::YELLOW},  {"!",  colors::YELLOW},            
    };


    const string storage_path = get_home_dir() + "/.config/scat/";
    DB database(storage_path + "sources.sqlite3");
    // std::cout << storage_path << "\n";
    // if (!database.isOpen()) {
        // auto dir = std::filesystem::create_directories(storage_path);
        // database.connect(database.getPath());
        // if (!database.isOpen()) {
            // ERROR("Could not load sqlite3 db on path \"" + storage_path + "\"");
            // exit(EXIT_FAILURE);
        // }
    // }

    database.execute(R"sql(SELECT rowid FROM KWD_CONT_STRING where rowid="0")sql", false, cs::default_);
    cs::database = &database;

    if (database.isNoSuchTableError())
    {
        std::cout << "nop table\n";
        database.dropAllTables();
        init_dbs(database);
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
            check_line(file, cs::KEYWORDS, cs::REP_KWDS, cs::KWDS_CONT, symbols);
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
            else if (option == "--save") {
                langData data = parse_file(path);
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
            if (option0 == "--create_ptr") {
                createLangPointer(database, option1, path);
                break;
            }

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

/*

function run {
    g++ $1 -o scat -std=c++17 -l sqlite3 &&
    ./scat test.py
}

*/