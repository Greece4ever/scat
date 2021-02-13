#include <sqlite3.h>
#include <iostream>
#include "sql.h"
#include "../colors.h"
#define ferr(x) std::cout << colors::RED << "[SQL Error]" << colors::ENDC << " " << x << "\n"

int callback(void *NotUsed, int argcount, char **argv, char **azColName) {
    return 0;
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

        bool execute(std::string sql_comamnd, bool print_err=true ) {
            *rcode = sqlite3_exec(this->database, sql_comamnd.c_str(), callback, 0, &err_msg);
            if (*rcode != SQLITE_OK) {
                if (print_err)
                    ferr(err_msg);
                return false;
            }
            return true;
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
            std::string query = R"sql(
                INSERT INTO KWD VALUES (NULL, %1, %2);
            )sql";

            escapeSQL(kwrd);

        }

};

int main() {
    // std::string unespaced = "takis\"; --\n DROP ALL DATABASES;";
    
    std::string unespaced = "\" \" \" \" \" myNewLineString \" \" \" \"; DROP TABLE SMB; \\n  ";
    DB database(".srcds"); SQL_INIT();

    escapeSQL(unespaced);

    database.execute(CREATE_COLLUMNS);
    database.commit();

    std::string sql = R"sql(INSERT INTO KWD VALUES (NULL, "def", 1))sql";
    database.execute(sql);
    database.commit();

    // bool r_value = database.execute(R"sql(
    //     INSERT INTO KWD ("ID", "KEYWORD", "COLOR")
    //     VALUES (NULL, "def", 1)
    //  )sql");

    // if (!r_value) {
        // std::cout << "Insert failed\n";
    // }

    // database.commit();

    return EXIT_SUCCESS;
}