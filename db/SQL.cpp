#include <sqlite3.h>
#ifndef _GLIBCXX_IOSTREAM
    #include <iostream>
#endif
#include "sql.h"
#include "../colors.h"
#ifndef _GLIBCXX_VECTOR
    #include <vector>
#endif
#ifndef ___MAIN___
    #define ___DEBUG___ 0xFFFFF & 0xCCC << 0b101
#endif
#define ___CALLBACK__ (void*,int,char**,char**)
#define ferr(x) std::cout << colors::RED << "[SQL Error]" << colors::ENDC << " " << x << "\n"

using std::vector;
using std::string;

int callback(void *NotUsed, int count, char **data, char **columns) {
    // int idx;

    // printf("There are %d column(s)\n", count);

    // for (idx = 0; idx < count; idx++) {
    //     printf("The data in column \"%s\" is: %s\n", columns[idx], data[idx]);
    // }

    // printf("\n");

    // return 0;
}

class DB
{
    /* Simple Wrapper arround the C sqlite3 API */
    private:
        sqlite3 *database;
        int  *rcode = new int;
        char *err_msg;
    public:
        DB(std::string path) {
            *rcode = sqlite3_open(".srcds", &database);    
            if (*rcode != SQLITE_OK) {
                exit(EXIT_FAILURE);
            }
        }


        bool execute(std::string sql_comamnd, 
            bool print_err=true,
            int (*onResponse)(void*, int, char**, char**)=callback) 
        {
            
            *rcode = sqlite3_exec(this->database, sql_comamnd.c_str(), onResponse, 0, &err_msg);
            if (*rcode != SQLITE_OK) {
                if (print_err)
                    ferr(err_msg);
                return false;
            }
            return true;
        }


        bool insert(std::string table_name, vector<string> values, bool print_err=true, int (*callback)___CALLBACK__=callback) {
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

    #ifdef ___DEBUG___
        sqlite3* getDB() {
            return this->database;
        }
    #endif
        
        ~DB() {
            delete rcode;
            sqlite3_free( err_msg );
            sqlite3_close( database );
        }
};

int callbackID(void *NotUsed, int count, char **data, char **columns) {
    int ID = std::stoi(data[0]);
    std::cout << "ID is " << ID << "\n"; 
    return ID;
}

int GetOrCrate(DB& database) {
    bool inserted = database.insert("KWD", {"typedef", "95"}, false);
    if (inserted) {
        database.execute(R"sql(SELECT last_insert_rowid())sql", false, callbackID);
        database.commit();
    }
    return 0;
}


int main() {    
    DB database(".srcds"); SQL_INIT();

    database.execute(CREATE_COLLUMNS);
    database.commit();

    char **collumns;
    char *err;
    sqlite3_exec(database.getDB(), R"sql(SELECT "rowid", "keyword", "color" FROM KWD where color=1)sql", callback, collumns, &err);
    // for (int i=0; i < )
    
    // std::string fetchSQL = R"sql(SELECT "KEYWORD", "COLOR" from KWD where KWD.color=1)sql";
    // database.execute(fetchSQL, true, callback);
    // GetOrCrate(database);
    // database.insert("KWD", {"def", "blue"});
    // database.commit();
    // database.execute(fetchSQL, true, );


    return EXIT_SUCCESS;
}