#include <sqlite3.h>
#ifndef _GLIBCXX_IOSTREAM
    #include <iostream>
#endif
#include "sql.h"
#include "../colors.h"
#ifndef _GLIBCXX_VECTOR
    #include <vector>
#endif
#define ___CALLBACK__ (void*,int,char**,char**)
#define ferr(x) std::cout << colors::RED << "[SQL Error]" << colors::ENDC << " " << x << "\n"
#define __LANG_PARSE__ 0
#include "./lang_parse.cpp"
#define SQL_GET_LAST_INSERT_ID "SELECT last_insert_rowid()"

using std::vector;
using std::string;

namespace cs {  
    vector<string> data;
    

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
}


class DB
{
    /* Simple Wrapper arround the C sqlite3 API */
    private:
        sqlite3 *database;
        int  *rcode = new int;
        char *err_msg = nullptr;
    public:
        DB(std::string path) {
            *rcode = sqlite3_open(path.c_str(), &database);    
            if (*rcode != SQLITE_OK) {
                exit(EXIT_FAILURE);
            }
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


typedef struct
{
    string LANG_ID;
    vector<string> KEYWORD_IDS;
    vector<string> SYMBOL_IDS;
} KWD_ID;


KWD_ID createLang(DB& database, langData data) {
    escapeSQL(data.file_extension);
    bool inserted =  database.insert("LANG", {data.file_extension}, false);
    if (!inserted) {
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

        database.execute(SQL_INSERT, true);
    }

    for (int i=0; i < id_data.SYMBOL_IDS.size(); i++) {
        string SQL_INSERT = R"sql(
            INSERT INTO LANG_RPT 
            VALUES (NULL, $1, $2)
        )sql";

        replaceOne(SQL_INSERT, "$1", id_data.LANG_ID);
        replaceOne(SQL_INSERT, "$2", id_data.SYMBOL_IDS[i]);
        database.execute(SQL_INSERT, true);
    }
}

int main() {    
    DB database("KEYWORDS.sqlite3");
    SQL_INIT();

    database.execute(CREATE_COLLUMNS);
    database.execute(ManyToMany);
    database.commit();

    langData data = parse_file("./cpp.lang");
    // printKwds(data);
    KWD_ID id_data = createLang(database, data);
    createLangKwdConenctions(database, id_data);


    return EXIT_SUCCESS;
}