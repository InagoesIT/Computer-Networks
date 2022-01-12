#pragma once

#define LINE_SIZE 600

class DbQueries
{
    sqlite3 * dbObj;

    void createDb();
    // -- callbacks --
    // process the output of getNLeaders
    static int getNLeadersCallback(void * sock, int colCount, char** colData, char** colName);
    // return the number of users in the database
    static int countCallback(void * number, int colCount, char** colData, char** colName);
    // used for verifying if the select statement returned true or false
    static int resultCallback(void * exists, int colCount, char** colData, char** colName); 

    public:
    DbQueries(string dbName);
    ~DbQueries();

    void addUser(string name, string password);
    bool isPasswordCorrect(string name, string password);
    bool isNameAvailable(string name);
    void incrementScore(string name);
    void getNLeaders(int sock, int n);
};

void DbQueries::createDb()
{
    char * dbErrMsg = 0;
    string create;

    // SQL statement for creating the users table if it doesn't exist
    create = "CREATE TABLE IF NOT EXISTS users("  \
      "name          VARCHAR(50)   PRIMARY KEY    NOT NULL," \
      "password      VARCHAR(50)                  NOT NULL," \
      "games_won     INT                          NOT NULL );" ;

    // execute SQL statement 
    sqlite3_exec(dbObj, create.c_str(), NULL, 0, &dbErrMsg);
   
    if(strcmp(sqlite3_errmsg(dbObj), "not an error"))
        cerr << "Can't create database: " << sqlite3_errmsg(dbObj) << endl;
}

int DbQueries::getNLeadersCallback(void * sock, int colCount, char** colData, char** colName) 
{
    char output[LINE_SIZE] = "<";
    strcat(output, colData[0]);
    strcat(output, "> has the score: ");
    strcat(output, colData[1]);
    strcat(output, "\n");
    write(*(int*)sock, output, LINE_SIZE);
    return 0;
}

int DbQueries::countCallback(void * number, int colCount, char** colData, char** colName)
{
    *(int*) number =  atoi(colData[0]);
    return 0;
}

int DbQueries::resultCallback(void * notUsed, int colCount, char** colData, char** colName) 
{
    // we didn't get anything from the select
    return strcmp(colData[0], "1") == 0;
}

DbQueries::DbQueries(string dbName)
{
    sqlite3_open(dbName.c_str(), &dbObj);
   
    if(strcmp(sqlite3_errmsg(dbObj), "not an error"))
        cerr << "Can't open database: " << sqlite3_errmsg(dbObj) << endl;

    createDb();
}

DbQueries::~DbQueries()
{
    sqlite3_close(dbObj);
}

void DbQueries::addUser(string name, string password)
{
    string add;

    add = "INSERT INTO users VALUES('" + name + "', '" + password + "', 0);";

    sqlite3_exec(dbObj, add.c_str(), NULL, 0, NULL);

    if(strcmp(sqlite3_errmsg(dbObj), "not an error"))
        cerr << "Can't add user: " << sqlite3_errmsg(dbObj) << endl;
}

bool DbQueries::isPasswordCorrect(string name, string password)
{
    string isUser;

    isUser = "SELECT CASE WHEN EXISTS (SELECT * FROM users WHERE name = '" + name 
                + "' AND password = '" + password + "') THEN 1 ELSE 0 END;";
 
    const auto exists {sqlite3_exec(dbObj, isUser.c_str(), resultCallback, NULL, NULL)};

    return exists;
}

bool DbQueries::isNameAvailable(string name)
{
    string isUser;

    isUser = "SELECT CASE WHEN EXISTS (SELECT * FROM users WHERE name = '" + name 
                + "') THEN 0 ELSE 1 END;";

    const auto exists {sqlite3_exec(dbObj, isUser.c_str(), resultCallback, NULL, NULL)};

    return exists;
}

void DbQueries::incrementScore(string name)
{
    string isUser;

    isUser = "UPDATE users SET games_won = games_won + 1  WHERE name = '" + name + "';";

    sqlite3_exec(dbObj, isUser.c_str(), NULL, NULL, NULL);    
}

void DbQueries::getNLeaders(int sock, int n)
{
    char * dbErrMsg = 0;
    string statement;

    statement = "SELECT COUNT(*) FROM users;";

    int size;
    sqlite3_exec(dbObj, statement.c_str(), countCallback, &size, NULL); 

    if (size < n)
        statement = "SELECT name, games_won FROM users ORDER BY 2 DESC;";
    else    
        statement = "SELECT name, games_won FROM users ORDER BY 2 DESC LIMIT " 
                + to_string(n) + ";";

    sqlite3_exec(dbObj, statement.c_str(), getNLeadersCallback, &sock, &dbErrMsg);   

    if(strcmp(sqlite3_errmsg(dbObj), "not an error"))
        cerr << dbErrMsg << endl;
}