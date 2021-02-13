#include <fstream>
#ifndef _STRING_H
    #include <string.h>
#endif
#ifndef _GLIBCXX_IOSTREAM
    #include <iostream>
#endif
#ifndef __COLOR__
    #include "../colors.h"
#endif
#ifndef ERROR
    #define ERROR(x) std::cout << colors::RED << "[Error] " << colors::ENDC << x << "\n"
#endif
#ifndef _GLIBCXX_VECTOR
    #include <vector>
#endif
#define __str__(x)  colors::GREEN + x + colors::ENDC
#define MODE_KWD     (unsigned char)0
#define MODE_REPKWD  (unsigned char)1

using std::string;
using std::vector;
using std::tuple;

#define KWD_MATRIX      vector<   tuple<string, int>  >
#define REP_KWD_MATRIX  vector< tuple< tuple<string, string>, int> >


/* Quite limiting, the standard library is... */

void trimForward(std::string& str) {
    if (str.size() == 0)
        return;
    while (str.size() > 0 && isspace(str[0]))
    {
        str.erase(str.begin());
    }
}


void trimBackward(std::string& str) {
    if (str.size() == 0)
        return;

    int size = str.size();
    while (size > 0 && isspace(str[size - 1])) {
        str.erase(str.begin() + size - 1);
        size = str.size();
    }
}


void trim(std::string& str) {
    trimForward(str);
    trimBackward(str);
}


std::string splitSpace(std::string& str) {
    std::string str1;
    int size = str.size();
    while (size > 0 && !isspace(str[0])) 
    {
        str1.push_back(str[0]);
        str.erase(str.begin());
        size = str.size();  
    }

    trimForward(str);
    return str1;
}

std::string toUpper(std::string& str) {
    std::string str1;
    for (int i=0; i < str.size(); i++) {
        str1.push_back(toupper(str[i]));
    }
    return str1;
}

typedef struct 
{
    KWD_MATRIX  kwd_colors;
    REP_KWD_MATRIX rep_kwd_colors;
    std::string file_extension;
} langData;

langData parse_file(std::string path) {
    using std::vector;
    using std::string;
    using std::tuple;

    std::ifstream file;
    file.open(path);
    if (!file.is_open()) {
        char* err = strerror( errno );
        ERROR("Could not parse file: " + (std::string)err);
        exit(EXIT_FAILURE);
    }

    KWD_MATRIX kwd_colors;
    REP_KWD_MATRIX rep_kwd_colors;


    string line;
    string file_extension;
    ushort line_count;


    while (getline(file, line)) {
        line_count++;
        trim(line);
        if (line == "")
            continue;

        file_extension = line;
        break;
    }

    if (file_extension.size() == 0) {
        ERROR("Empty file");
        exit(EXIT_FAILURE);
    }

    unsigned char mode = MODE_KWD;

    while (getline(file, line)) {
        line_count++;
        trim(line);
        if (line == "") 
            continue;

        if (line.size() == 4) {
            std::string upr = toUpper(line);
            if (upr == "KWD:") {
                mode = MODE_KWD;
                continue;
            }
            else if (upr == "REP:") {
                mode = MODE_REPKWD;
                continue;
            }
            
        }

        if (mode == MODE_KWD) {
            string back = line;
                string front = splitSpace(back);

            string upperColor = toUpper(back);

            try {
                int code = colors::SIcodes.at( upperColor );
                kwd_colors.push_back(
                    {front, code}
                );
            }
            catch (std::out_of_range) {
                ERROR("Line: " + std::to_string(line_count) + " (\"" + __str__(front) + "\", \"" + __str__(back) + "\")," + " Color \"" + __str__(upperColor) + "\"" + " not found.");
                exit(EXIT_FAILURE);
            }


        }
        else if (mode == MODE_REPKWD) {
            string back = line;                // COLOR
            string front = splitSpace(back);   // KEYWORD
            string middle = splitSpace(back);  // NED KEYWORD

            string upperColor = toUpper(back);

            try {
                int code = colors::SIcodes.at( upperColor );

                rep_kwd_colors.push_back(
                    { {front, middle}, code }
                );
            } 
            catch (std::out_of_range) {
                ERROR("Color \"" + upperColor + "\"" + "not found.");
                exit(EXIT_FAILURE);
            }
        }

    }
    
    langData data;
    data.kwd_colors = kwd_colors;
    data.rep_kwd_colors = rep_kwd_colors;
    data.file_extension = file_extension;

    return data;
}


void printKwds(langData data) {
    auto kwd_colors =     data.kwd_colors;
    auto rep_kwd_colors = data.rep_kwd_colors;
    auto file_extension = data.file_extension;

    std::cout << "File extension \"" << colors::BOLD << file_extension <<colors::ENDC << "\"\n";

    std::cout << "Keywords:\n";
    for (ushort i=0; i < kwd_colors.size(); i++) {
        string kwd = std::get<0>(kwd_colors[i]);
        int ccode =  std::get<1>(kwd_colors[i]);

        string color = colors::IScodes.at(ccode);
        std::cout << "\t" << color;
        std::cout << kwd << colors::ENDC << "\n";
    }

    std::cout << "Repeating Keywords:\n";

    for (ushort i=0; i < rep_kwd_colors.size(); i++) {
        string start = std::get<0>(std::get<0>(rep_kwd_colors[i]));
        string end = std::get<1>(std::get<0>(rep_kwd_colors[i]));

        int ccode = std::get<1>(rep_kwd_colors[i]);
        string color = colors::IScodes.at(ccode);
        std::cout << color;
        std::cout << "\"" << start << "\"\t\"" << end << "\"" << colors::ENDC << "\n";
    }
}


int main() {
    langData data = parse_file("./test.lang");
    printKwds(data);
    return 0;
}