#include <sqlite3.h>
#include <iostream>
#include "sql.h"


int callback(void *NotUsed, int argcount, char **argv, char **azColName) {
    return 0;
}

class DB
{
    private:
        sqlite3 *database;
        int  *rcode   = new int;
        char *err_msg = new char;
    public:
        DB(std::string path) {
            *rcode = sqlite3_open(".srcds", &database);    
            if (*rcode != SQLITE_OK) {
                exit(EXIT_FAILURE);
            }
        }

        void execute(std::string sql_comamnd) {
            *rcode = sqlite3_exec(this->database, sql_comamnd.c_str(), callback, 0, &err_msg);
            if (*rcode != SQLITE_OK) {
                std::cout << err_msg << "\n";
                exit(EXIT_FAILURE);
            }
        }

        void commit() {
            sqlite3_exec(this->database, "END TRANSACTION;", NULL, NULL, NULL);
        }
        
        ~DB() {
            delete err_msg;
            delete rcode;
            sqlite3_close( database );
        }
};

int main() {
    DB database(".srcds");
    replaceAll(CREATE_COLLUMNS, "%STRUCT", KWD_STRUCT);

    database.execute(CREATE_COLLUMNS);
    database.commit();

    
    // std::string insrt = R"sql(
    //     INSERT INTO KWDCOLRS ("ID", "KEYWORD", "COLOR") VALUES (NULL, "def", 1) 
    // )sql";

    // database.execute(insrt);

    return EXIT_SUCCESS;
}