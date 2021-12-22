#pragma once

class DbQueries
{
    sqlite3 * dbObj;

    void createDb();
    // -- callbacks --
    // process the output of getNLeaders
    static int getNLeadersCallback(void * notUsed, int colCount, char** colData, char** colName);
    // return the number of users in the database
    static int countCallback(void * notUsed, int colCount, char** colData, char** colName);
    // used for verifying if the select statement returned true or false
    static int resultCallback(void * exists, int colCount, char** colData, char** colName); 

    public:
    DbQueries(string dbName);
    ~DbQueries();

    void addUser(string name, string password);
    bool isPasswordCorrect(string name, string password);
    bool isNameAvailable(string name);
    void incrementScore(string name);
    bool getNLeaders(int n);
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

int DbQueries::getNLeadersCallback(void * notUsed, int colCount, char** colData, char** colName) 
{
    cout << "<" << colData[0] << ">";
    cout << " has the score: " << colData[1];

    cout << endl;
    return 0;
}

int DbQueries::countCallback(void * notUsed, int colCount, char** colData, char** colName)
{
    return atoi(colData[0]);
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

bool DbQueries::getNLeaders(int n)
{
    char * dbErrMsg = 0;
    string statement;

    statement = "SELECT COUNT(*) FROM users;";

    const auto number {sqlite3_exec(dbObj, statement.c_str(), countCallback, NULL, NULL)}; 

    if (number < n)
        return false;

    statement = "SELECT name, games_won FROM users ORDER BY 2 DESC LIMIT " 
                + to_string(n) + ";";

    sqlite3_exec(dbObj, statement.c_str(), getNLeadersCallback, NULL, &dbErrMsg);   

    if(strcmp(sqlite3_errmsg(dbObj), "not an error"))
        cerr << dbErrMsg << endl;

    return true;
    /* will use this to redirect stdout to socket:
    close(1);
    if( dup2((sock desc), 1) == -1)
        err;
    */
}