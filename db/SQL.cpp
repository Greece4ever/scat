#include <sqlite3.h>
#include <iostream>
#include "sql.h"
#include "../colors.h"
#define ___CALLBACK__ int (*callback)(void*,int,char**,char**)
#define ferr(x) std::cout << colors::RED << "[SQL Error]" << colors::ENDC << " " << x << "\n"

int callback(void *NotUsed, int argcount, char **argv, char **azColName) {
    return 1;
}

int add(int a, int b) {
    return a + b;
}

void test(int (*add)(int, int))  {
    std::cout << add(1, 2) << "\n";
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


        void insert(std::string table_name, int argc, std::string values[]) {
            std::string statement = "INSERT INTO " + table_name + " VALUES (NULL, " ;
            int i = 0;
            for (i=0; i < argc - 1; i++) {
                escapeSQL(values[i]);
                statement += + "\"" + values[i] + "\", ";
            }
            statement += + "\"" + values[i] + "\"";

            statement.push_back(')');
            statement.push_back(';');
            this->execute(statement);
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


class KeywordDB 
{
    DB *database;
    public:
        KeywordDB() {
            *database = DB("./aout");
        }

        void createKeyword(std::string kwrd, std::string color) {
            DB db = *database;
            std::string createSQL = R"sql(INSERT INTO KWD VALUES (NULL, "%1", %2))sql";
            escapeSQL(kwrd);
            replaceOne(createSQL, "%1", kwrd);
            std::cout << createSQL << "\n";

//          std::string fetchSQL = R"sql(SELECT rowid from KWD WHERE KWD.KEYWORD="def" AND KWD.COLOR=1)sql";

        }

};

int callback() {

}

int main() {    

    test(add);
    exit(EXIT_SUCCESS);
    DB database(".srcds"); SQL_INIT();

    database.execute(CREATE_COLLUMNS);
    database.commit();


    // std::string fetchSQL = R"sql(SELECT ("rowid") from KWD where KWD.color=1)sql";
    // database.execute(fetchSQL, true, );


    return EXIT_SUCCESS;
}