/* Parse a .lang file containg syntax highlighting rules */
#ifndef __LANG_PARSE__
    #define __LANG_PARSE__ 1
#endif
#ifndef _GLIBCXX_FSTREAM
    #include <fstream>
#endif
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
using std::string;
using std::vector;
using std::tuple;
using std::array;

#define __str__(x)  colors::GREEN + x + colors::ENDC
#define MODE_KWD      (unsigned char)0
#define MODE_REPKWD   (unsigned char)1
#define MODE_KWD_CONT (unsigned char)2


#define KWD_MATRIX      vector<   tuple<string, int>  >
#define REP_KWD_MATRIX  vector<   tuple< tuple<string, string>, int> >
#define KWD_CONT_MATRIX vector<tuple<array<string, 2>, string, vector<string>>>

#ifndef ___MAIN___
    #define ___DEBUG___ 0xFFFFF & 0xCCC << 0b101
#endif

tuple<array<string, 2>, string, vector<string>> parse_kwd_cont(std::string& line, ushort count);


/* Quite limiting, the standard library is... */
void trimForward(std::string& str) {
    while (str.size() > 0 && isspace(str[0]))
    {
        str.erase(str.begin());
    }
}


void trimBackward(std::string& str) {
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


string findExtension(string& str) {
    string extension;
    for (ushort i=str.size() - 1; i > 0; i--) {
        extension.insert(0, 1, str[i]);
        if (str[i] == '.')
            break;
    }
    return extension;
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
    KWD_CONT_MATRIX kwd_cont;
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
    KWD_CONT_MATRIX kwd_cont_colors;


    string line;
    string file_extension;
    ushort line_count = 0; // NOTE TODO BUG


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
    if ((int)file_extension.find(".") == -1) {
        ERROR("No \".\" found in file extension \"" + file_extension + "\"");
        ERROR("Try something like \"." + file_extension + "\"");
        exit(EXIT_FAILURE);
    }

    unsigned char mode = MODE_KWD;

    while (getline(file, line)) {
        line_count++;
        trim(line);
        if (line == "") 
            continue;

        if (line.size() >= 4) {
            std::string upr = toUpper(line);

            if (upr == "KWD:") {
                mode = MODE_KWD;
                continue;
            }
            else if (upr == "REP:") {
                mode = MODE_REPKWD;
                continue;
            }
            else if (upr == "KWD_CONT:") {
                mode = MODE_KWD_CONT;
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
                ERROR("Line: " + std::to_string(line_count) + " (\"" + __str__(front) + "\", \"" + __str__(back) + "\")," + " Color \"" + __str__(upperColor) + "\"" + " not found.");
                exit(EXIT_FAILURE);
            }
        }
        else if (mode == MODE_KWD_CONT) {
            kwd_cont_colors.push_back( parse_kwd_cont(line, line_count) );
        }
    }
    
    langData data;
        data.kwd_colors = kwd_colors;
        data.rep_kwd_colors = rep_kwd_colors;
        data.kwd_cont = kwd_cont_colors;     
        data.file_extension = file_extension;

    return data;
}


void printKwds(langData data) {
    auto& kwd_colors =     data.kwd_colors;
    auto& rep_kwd_colors = data.rep_kwd_colors;
    auto& kwd_cont = data.kwd_cont;
    auto& file_extension = data.file_extension;


    std::cout << "File extension \"" << colors::BOLD << file_extension <<colors::ENDC << "\"\n";

    std::cout << "\tKeywords:\n";
    for (ushort i=0; i < kwd_colors.size(); i++) {
        string kwd = std::get<0>(kwd_colors[i]);
        int ccode =  std::get<1>(kwd_colors[i]);

        string color = colors::IScodes.at(ccode);
        std::cout << "\t\t" << color;
        std::cout << kwd << colors::ENDC << "\n";
    }

    std::cout << "\tRepeating Keywords:\n";

    for (ushort i=0; i < rep_kwd_colors.size(); i++) {
        string start = std::get<0>(std::get<0>(rep_kwd_colors[i]));
        string end = std::get<1>(std::get<0>(rep_kwd_colors[i]));

        int ccode = std::get<1>(rep_kwd_colors[i]);
        string color = colors::IScodes.at(ccode);
        std::cout << "\t\t" << color;
        std::cout << "\"" << start << "\"\t\"" << end << "\"" << colors::ENDC << "\n";
    }

    std::cout << "\tContinuing Keywords:\n";


    for (ushort i=0; i < kwd_cont.size(); i++)
    {
        auto __colors  = std::get<0>(kwd_cont[i]);
        string keyword =  std::get<1>(kwd_cont[i]);
        vector<string> delimeters = std::get<2>(kwd_cont[i]);
        ACTIVATE(colors::IScodes.at( std::stoi(  __colors[0] ) ));
            std::cout << "\t\t" << keyword << "\t";
        DEACTIVATE();
        uint k;
        ACTIVATE( colors::IScodes.at( std::stoi( __colors[1] ) )  );
            for (k=0; k < delimeters.size() - 1; k++) 
            {
                std::cout << delimeters[i] << ", ";
            }
            std::cout << delimeters[k] << "\n";
        DEACTIVATE();
    }
    DEACTIVATE();

}


vector<string> split(const string& str, const string& delim)
{
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token =  str.substr(prev, pos-prev);
            /* custom */ trim(token);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

tuple<array<string, 2>, string, vector<string>> parse_kwd_cont(std::string& line, ushort line_count) {
        trim(line);

        std::string keyword =  splitSpace(line);
            std::string color0  =  splitSpace(line);
            std::string color1  =  splitSpace(line);

            try {
                color0 = std::to_string( colors::SIcodes.at(toUpper(color0)) );
                color1 = std::to_string( colors::SIcodes.at(toUpper(color1)) );
            }
            catch (std::out_of_range) {
                ERROR("Line " + std::to_string(line_count) +  ": Color \"" + __str__(color0) + "\" or \"" + __str__(color1) + "\"" + " not found.");
                exit(EXIT_FAILURE);
            }

        vector<string> data= split(line, ",");
        return { {color0, color1}, keyword, data };
}


#ifdef ___DEBUG___ 
    #if __LANG_PARSE__
        int main() {
            langData data = parse_file("./cpp.lang");
            printKwds(data);
            return EXIT_SUCCESS;
        }
    #endif
#endif
