#ifndef _GLIBCXX_STRING
    #include <string>
#endif

std::string KWD_STRUCT = R"sql(
    "ID"      INT PRIMARY KEY,
    "KEYWORD" TEXT NOT NULL,
    "COLOR"   INT NOT NULL,
    CONSTRAINT unq UNIQUE ("KEYWORD", "COLOR")        
)sql";

std::string CREATE_COLLUMNS = R"sql(
    -- Fully matched keywords (fully matched words) with certain color
    CREATE TABLE IF NOT EXISTS KWD(
        %STRUCT
    );

    -- Repeating chars (waiting for something to end like comment with "\n")
    CREATE TABLE IF NOT EXISTS RPT(
        %STRUCT
    );

    -- Symbols that do not need to be fullmatch (e.g "+", "-" etc)
    CREATE TABLE IF NOT EXISTS SMB(
        %STRUCT
    );

    -- Language with many keywords with colors
    CREATE TABLE IF NOT EXISTS LANG(
        "ID"       INT PRIMARY KEY,
        "NAME" TEXT UNIQUE NOT NULL,

        "KWDS" REFERENCES KWDCOLRS(ID),
        "RPTS" REFERENCES RPT(ID),
        "SMBS" REFERENCES SMB(ID)
    );

    -- Languages that point to other languages
    CREATE TABLE IF NOT EXISTS LANG_PTR(
        "ID"       INT PRIMARY KEY,
        "NAME"     TEXT UNIQUE NOT NULL,
        "LANG_ID"  REFERENCES LANG(ID)
    );  
)sql";

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}