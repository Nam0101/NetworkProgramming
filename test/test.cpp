#include <iostream>
#include <sqlite3.h>

int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    for (int i = 0; i < argc; i++)
    {
        std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << '\n';
    }
    std::cout << "------------------------\n";
    return 0;
}

int main()
{
    sqlite3 *db;
    char *errMsg = 0;

    int rc = sqlite3_open("test.db", &db);

    if (rc)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Opened database successfully" << std::endl;
    }
    const char* create="CREATE TABLE user (id INT PRIMARY KEY NOT NULL, name TEXT NOT NULL, age INT NOT NULL, address CHAR(50), salary REAL );";
    rc = sqlite3_exec(db, create, callback, 0, &errMsg);
    const char* insert="INSERT INTO user (id, name, age, address, salary) VALUES (1, 'Paul', 32, 'California', 20000.00 );";
    rc = sqlite3_exec(db, insert, callback, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    else
    {
        std::cout << "Records created successfully" << std::endl;
    }

    // SQL statement to select all rows from the 'user' table
    const char *sql = "SELECT * FROM user;";

    // Execute the SQL statement and pass the callback function
    rc = sqlite3_exec(db, sql, callback, 0, &errMsg);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);

    return 0;
}
