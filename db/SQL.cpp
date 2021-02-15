#ifndef SQLITE3_H
    #include <sqlite3.h>
#endif
#ifndef _GLIBCXX_IOSTREAM
    #include <iostream>
#endif
#include "sql.h"
#ifndef __COLOR__
    #include "../colors.h"
#endif
#ifndef _GLIBCXX_VECTOR
    #include <vector>
#endif
#define ___CALLBACK__ (void*,int,char**,char**)
#define ferr(x) std::cout << colors::RED << "[SQL Error]" << colors::ENDC << " " << x << "\n"
#define __LANG_PARSE__ 0
#include "./lang_parse.cpp"
#define SQL_GET_LAST_INSERT_ID "SELECT last_insert_rowid()"
#ifndef _GLIBCXX_ARRAY
    #include <array>
#endif

using std::vector;
using std::string;


namespace cs {  
    vector<string> data;
    vector<std::array<string, 2>> KEYWORDS;
    vector<std::array<string, 3>> REP_KWDS;    

    int default_(void *NotUsed, int count, char **data, char **columns) {
        return EXIT_SUCCESS;
    }


    int callback(void *NotUsed, int count, char **data, char **columns) {
        vector<string>().swap( cs::data );
        for (int i=0; i < count; i++) {
            // std::cout << columns[i] << ": " << data[i] << "\n";
            cs::data.push_back(data[i]);
        }

        return 0;
    }

    #ifdef ___DEBUG___
        int test(void *NotUsed, int count, char **data, char **columns) {
            int idx;

            printf("There are %d column(s)\n", count);

            for (idx = 0; idx < count; idx++) {
                printf("The data in column \"%s\" is: %s\n", columns[idx], data[idx]);
            }

            printf("\n");
            return 0;
        }
    #endif

    int insertCallback(void *NotUsed, int count, char **data, char **columns) {
        for (int i=0; i < count; i++) {
            cs::data.push_back(data[i]);
        }
        return EXIT_SUCCESS;
    }

    int KeyWordInsertCallback(void *NotUsed, int count, char **data, char **columns) {
        cs::KEYWORDS.push_back({data[0], colors::SScodes.at(data[1])});
        return EXIT_SUCCESS;
    }

    // colors::bquotes

    int RptKwdInsertionCallback(void *NotUsed, int count, char **data, char **columns) {
        string s1 = data[1];
        if (s1 == (std::string)"\\n") // Really...
            s1 = "\n";
        cs::REP_KWDS.push_back({data[0], s1, colors::SScodes.at(data[2]) });
        return EXIT_SUCCESS;
    }

}


class DB
{
    /* Simple Wrapper arround the C sqlite3 API */
    private:
        sqlite3 *database;
        int  *rcode = new int;
        char *err_msg = nullptr;
        bool *is_open = new bool;
        string *path_ = new string;
    public:
        DB(std::string path) {
            *this->path_ = path;
            this->connect(path);
        }

        void connect(std::string path) {
            *rcode = sqlite3_open(path.c_str(), &database);    
            if (*rcode != SQLITE_OK) {
                *is_open = false; 
                return;
            }
            *is_open = true;            
        }

        bool isOpen() {
            return *is_open;
        }

        string getPath() {
            return *this->path_;
        }

        bool isNoSuchTableError() {
            if (err_msg == nullptr)
                return false;
            string sterr = err_msg;
            string err = toUpper(sterr);
            int found = err.find("NO SUCH TABLE:");
            return found != -1;
        }

        bool execute(std::string sql_comamnd, 
            bool print_err=true,
            int (*onResponse)(void*, int, char**, char**)=cs::default_) 
        {
            
            *rcode = sqlite3_exec(this->database, sql_comamnd.c_str(), onResponse, 0, &err_msg);
            if (*rcode != SQLITE_OK) {
                if (print_err)
                    ferr(err_msg);
                return false;
            }
            return true;
        }


        bool insert(std::string table_name, vector<string> values, bool print_err=true, int (*callback)___CALLBACK__=cs::default_) {
            std::string statement = "INSERT INTO " + table_name + " VALUES (NULL, " ;
            int i = 0;
            int argc = values.size();
            for (i=0; i < argc - 1; i++) {
                escapeSQL(values[i]);
                statement += + "\"" + values[i] + "\", ";
            }
            statement += + "\"" + values[i] + "\"";

            statement.push_back(')');
            statement.push_back(';');

            return this->execute(statement, print_err, callback);
        }



        void commit() {
            sqlite3_exec(this->database, "END TRANSACTION;", NULL, NULL, NULL);
        }
        
        ~DB() {
            delete rcode;
            delete path_;
            delete is_open;
            sqlite3_free( err_msg );
            sqlite3_close( database );
        }
};


string insertAndGetID(DB& database, string kwd, string color) {
    bool inserted = database.insert("KWD", {kwd, color}, false);
    if (!inserted) {
        string query = R"sql(
            SELECT "rowid" FROM KWD
            WHERE COLOR=$1 and KEYWORD="$2";
        )sql";

        escapeSQL(kwd);
        
        replaceOne(query, "$1", color);
        replaceOne(query, "$2", kwd);
        database.execute(query, true, cs::callback);
        return cs::data[0];   
    }
    database.execute(SQL_GET_LAST_INSERT_ID, true, cs::callback);
    return cs::data[0];
}


string insertSymbolAndGetId(DB& database, string STARTING_KEYWORD, string ENDING_KEYWORD, string color)
{
    bool inserted = database.insert("RPT", {STARTING_KEYWORD, ENDING_KEYWORD, color}, false);
    if (!inserted) {
        string query = R"sql(
            SELECT "rowid" FROM RPT
            WHERE RPT.STARTING_KEYWORD="$1" AND RPT.ENDING_KEYWORD="$2"
            AND RPT.COLOR=$3;
        )sql";

        escapeSQL(STARTING_KEYWORD);
        escapeSQL(ENDING_KEYWORD);

        replaceOne(query, "$1", STARTING_KEYWORD);
        replaceOne(query, "$2", ENDING_KEYWORD);
        replaceOne(query, "$3", color);

        database.execute(query, true, cs::callback);
        return cs::data[0];
    }
    database.execute(SQL_GET_LAST_INSERT_ID, true, cs::callback);
    return cs::data[0];
}



void init_dbs(DB& database) {
    SQL_INIT();
    database.execute(CREATE_COLLUMNS);
    database.execute(ManyToMany);
    database.commit();
}


typedef struct
{
    string LANG_ID;
    vector<string> KEYWORD_IDS;
    vector<string> SYMBOL_IDS;
} KWD_ID;


KWD_ID createLang(DB& database, langData data) {
    // escapeSQL(data.file_extension);
    bool inserted =  database.insert("LANG", {data.file_extension}, false);
    if (!inserted) {
        if (database.isNoSuchTableError()) {
            init_dbs(database);
            return createLang(database, data);
        }
        WARNING("Language \"" + data.file_extension + "\" already exists, this will add even more keywords");
        WARNING("if you wish to insert tottaly new ones, delete it");

        string QUERY = R"sql(
            SELECT "rowid" FROM LANG
            WHERE LANG.NAME="$1"
        )sql";
        replaceOne(QUERY, "$1", data.file_extension);
        database.execute(QUERY, true, cs::callback);
    }
    else 
        database.execute(SQL_GET_LAST_INSERT_ID, true, cs::callback);


    string langID = cs::data[0];
    vector<string> KEYWORD_IDS;
    vector<string> SYMBOL_IDS;


    for (int i=0; i < data.kwd_colors.size(); i++) {
        string KEYWORD = std::get<0>(data.kwd_colors[i]);
        string COLOR = std::to_string( std::get<1>(data.kwd_colors[i]) );
        KEYWORD_IDS.push_back( insertAndGetID(database, KEYWORD, COLOR) );
    }

    for (int i=0; i < data.rep_kwd_colors.size(); i++) {
        string color = std::to_string ( std::get<1>( data.rep_kwd_colors[i] ) );
        string start_kwd  = std::get<0> ( std::get<0>( data.rep_kwd_colors[i] )  );
        string endng_kwd  = std::get<1> ( std::get<0>( data.rep_kwd_colors[i] )  );
        SYMBOL_IDS.push_back(  insertSymbolAndGetId(database, start_kwd, endng_kwd, color)  );
    }


    KWD_ID id_data;
    id_data.KEYWORD_IDS = KEYWORD_IDS;
    id_data.SYMBOL_IDS = SYMBOL_IDS;
    id_data.LANG_ID = langID;
    return id_data;

}


void createLangKwdConenctions(DB& database, KWD_ID id_data) {
    for (int i=0; i < id_data.KEYWORD_IDS.size(); i++) {
        string SQL_INSERT = R"sql(
            INSERT INTO LANG_KWD 
            VALUES (NULL, $1, $2)
        )sql";

        replaceOne(SQL_INSERT, "$1", id_data.LANG_ID);
        replaceOne(SQL_INSERT, "$2", id_data.KEYWORD_IDS[i]);

        database.execute(SQL_INSERT, false);
    }

    for (int i=0; i < id_data.SYMBOL_IDS.size(); i++) {
        string SQL_INSERT = R"sql(
            INSERT INTO LANG_RPT 
            VALUES (NULL, $1, $2)
        )sql";

        replaceOne(SQL_INSERT, "$1", id_data.LANG_ID);
        replaceOne(SQL_INSERT, "$2", id_data.SYMBOL_IDS[i]);
        database.execute(SQL_INSERT, false); // Note errors are pointless -> database.execute(SQL_INSERT, true);
    }
}

const string comma = ",";

string VecToString(vector<string>& vect) {
    string arr = "(";
    int i = 0;
    for (i=0; i < vect.size() - 1; i++) {
        arr += vect[i] + ",";
    }

    arr += vect[i];
    arr.push_back(')');
    return arr;
}

/* Set the value of
    cs::KEYWORDS and
    cs::REP_KWDS
    to the array of keyword colors
    and the array of repeated keyword colors
 */
void get_kwds(DB& database, string language, string language0="") 
{
    // Bad Code alert
    escapeSQL(language);
    
    string QUERY = R"sql(
        SELECT "rowid" from
        LANG WHERE LANG.NAME="$1"
    )sql";

    replaceOne(QUERY, "$1", language);
    database.execute(QUERY, true, cs::callback);
    
    if(database.isNoSuchTableError()) {
        init_dbs(database);
        ERROR("No Default keyword colorset found, exiting. (DB initialised for the first time)");
        exit(EXIT_FAILURE);
        return get_kwds(database, language);
    }

    if (cs::data.size() == 0) {
        if (language == ".default") {
            ERROR("Language \"" + language0 + "\" not found and no default was set.");
            ERROR("A default can be set by running --default and then the path.");
            exit(EXIT_FAILURE);
        }
        return get_kwds(database, ".default", language); // Return the default language
    }
    string langID = cs::data[0];

    string FQUERY = R"sql(
        SELECT "KWD" FROM
        LANG_KWD WHERE LANG=$1;
    )sql";

    replaceOne(FQUERY, "$1", langID);
    database.execute(FQUERY, true, cs::insertCallback);

    string KWDQUERY = R"sql(
        SELECT "KEYWORD", "COLOR" FROM KWD
        WHERE KWD.ROWID in $1
    )sql";

    
    replaceOne(KWDQUERY, "$1", VecToString(cs::data));
    database.execute(KWDQUERY, true, cs::KeyWordInsertCallback);


    vector<string>().swap( cs::data );

    string FQUERY2 =R"sql(
        SELECT "RPT" FROM LANG_RPT
        WHERE LANG_RPT.LANG=$1;
    )sql";
    
    replaceOne(FQUERY2, "$1", langID);
    database.execute(FQUERY2, true, cs::insertCallback);
    
    string FQUERY3 = R"sql( 
        SELECT  "STARTING_KEYWORD", "ENDING_KEYWORD","COLOR"
        FROM RPT WHERE RPT.ROWID in $1
    )sql";

    replaceOne(FQUERY3, "$1", VecToString(cs::data));
    database.execute(FQUERY3, true, cs::RptKwdInsertionCallback);
}


void deleteLang(DB& database, string language) {
    string query;
    query = R"sql(
        SELECT "rowid" FROM LANG
        WHERE LANG.NAME="$1";
    )sql";

    escapeSQL(language);
    replaceOne(query, "$1", language);
    database.execute(query, true, cs::insertCallback);
    
    if (cs::data.size() == 0) {
        ERROR("Language \"" + language + "\" not found.");
        exit(EXIT_FAILURE);
    }

    string langID = cs::data[0];

    string(R"sql(
        DELETE FROM LANG_KWD
        WHERE LANG_KWD.LANG=$1
    )sql").swap( query );

    replaceOne(query, "$1", langID);
    database.execute( query  );

    string(R"sql(
        DELETE FROM LANG_RPT
        WHERE LANG_RPT.LANG=$1
    )sql").swap( query );

    replaceOne(query, "$1", langID);
    database.execute( query );

    string(R"sql(
        DELETE FROM LANG
        WHERE LANG.NAME="$1"
    )sql").swap( query );

    replaceOne(query, "$1", language);
    database.execute(  query );
}


#ifndef ___MAIN___
    int main() {    
        DB database("KEYWORDS.sqlite3");
        // langData data = parse_file("./cpp.lang");
        // printKwds(data);
        KWD_ID id_data = createLang(database, data);
        createLangKwdConenctions(database, id_data);
        get_kwds(database, ".cpp");
        for (int i=0; i < cs::KEYWORDS.size(); i++) {
            std::cout << cs::KEYWORDS[i][1] << cs::KEYWORDS[i][0] << colors::ENDC << "\n";
        }

        for (int i=0; i < cs::REP_KWDS.size(); i++) {
            std::cout << cs::REP_KWDS[i][2] << cs::REP_KWDS[i][0] << " " << cs::REP_KWDS[i][1] << colors::ENDC << "\n";
        }


        return EXIT_SUCCESS;
    }
#endif
