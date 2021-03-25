#ifndef _GLIBCXX_STRING
    #include <string>
#endif


std::string LangManyToMany = R"sql(
    "ID"    INT PRIMARY KEY,
    "LANG"  REFERENCES LANG(ID),
)sql";

std::string CREATE_COLLUMNS = R"sql(
    -- Fully matched keywords (fully matched words) with certain color
    CREATE TABLE IF NOT EXISTS KWD(
        "ID"      INT PRIMARY KEY,
        "KEYWORD" TEXT NOT NULL,
        "COLOR"   INT NOT NULL,
        CONSTRAINT unq UNIQUE ("KEYWORD", "COLOR")        

    );

    -- Repeating chars (waiting for something to end like comment with "\n")
    CREATE TABLE IF NOT EXISTS RPT(
        "ID"      INT PRIMARY KEY,
        "STARTING_KEYWORD" TEXT NOT NULL,
        "ENDING_KEYWORD" TEXT NOT NULL,    
        "COLOR"   INT NOT NULL,
        CONSTRAINT unq UNIQUE ("STARTING_KEYWORD", "ENDING_KEYWORD", "COLOR")        
    );


    -- Language with many keywords with colors
    CREATE TABLE IF NOT EXISTS LANG(
        "ID"   INT PRIMARY KEY,
        "NAME" TEXT UNIQUE NOT NULL
    );

    CREATE TABLE IF NOT EXISTS KWD_CONT(
        "ID"      INT PRIMARY KEY,
        "LANG_ID"  REFERENCES LANG(ID),
        "COLOR0"   INT NOT NULL,
        "COLOR1"   INT NOT NULL,
        "STARTING_KEYWORD" TEXT NOT NULL,
        CONSTRAINT unq UNIQUE   ("LANG_ID", "COLOR0", "COLOR1", "STARTING_KEYWORD")
    );


    CREATE TABLE IF NOT EXISTS KWD_CONT_STRING(
        "ID" INT PRIMARY KEY,
        "KWD_CONT_ID" REFERENCES KWD_CONT(ID),
        "TEXT" TEXT NOT NULL,
        CONSTRAINT unq UNIQUE ("KWD_CONT_ID", "TEXT")
    );


    -- Languages that point to other languages
    CREATE TABLE IF NOT EXISTS LANG_PTR(
        "ID"       INT PRIMARY KEY,
        "NAME"     TEXT UNIQUE NOT NULL,
        "LANG_ID"  REFERENCES LANG(ID),
        CONSTRAINT unq UNIQUE ("NAME", "LANG_ID")
    );  
)sql";


std::string ManyToMany = R"sql(
    CREATE TABLE IF NOT EXISTS LANG_KWD(
      %MANY_TO_MANY
      "KWD"   REFERENCES KWD( ID ),
      CONSTRAINT unq UNIQUE ("LANG", "KWD")
    );


    CREATE TABLE IF NOT EXISTS LANG_RPT(
      %MANY_TO_MANY
      "RPT"   REFERENCES RPT( ID ),
      CONSTRAINT unq UNIQUE ("LANG", "RPT")
    );    

    CREATE TABLE IF NOT EXISTS LANG_KWD_CONT(
      %MANY_TO_MANY
      "KWD_CONT"   REFERENCES KWD_CONT( ID ),
      CONSTRAINT unq UNIQUE ("LANG", "KWD_CONT")
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

void replaceOne(std::string& str, std::string from, std::string to) {
    int index = str.find(from);
    if (index == -1)
        return;
    str.replace(index, from.size(), to);
}

void SQL_INIT() {
    replaceAll(ManyToMany, "%MANY_TO_MANY", LangManyToMany);
}

void escapeChar(std::string& sql_statement, std::string chr, std::string escaped) {
    /* Escape char at every occurance of it */
    int pos = 0;
    int index = sql_statement.find(chr, pos);

    while (index != -1)
    {
        sql_statement.replace(index, chr.length(), escaped);
        pos = index + 2;
        index = sql_statement.find(chr, pos);
    }    
}

const unsigned char es_char_count = 11; 

void escapeSQL(std::string& sql_str) {
    // if inserting with double quotes(") only single(') quotes have to be duplicated
    // escapeChar(sql_str, "'", "''"); 
    escapeChar(sql_str, "\"", "\"\"");
    // escapeChar(sql_str, "\\", "\\\\");
}
