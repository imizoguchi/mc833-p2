#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 

void save_movies();

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int main(int argc, char* argv[])
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int  rc;
   char *sql;

   /* Open database */
   rc = sqlite3_open("mc833.db", &db);
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stdout, "Opened database successfully\n");
   }

   /* Create SQL statement */
   sql = "CREATE TABLE MOVIE("  \
         "ID INT PRIMARY KEY     NOT NULL," \
         "TITLE          TEXT    NOT NULL," \
         "LAUNCHDATE     TEXT    NOT NULL," \
         "GENRE          TEXT    NOT NULL," \
         "COPIES         INT );";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Table created successfully\n");
   }
   sqlite3_close(db);
   save_movies();
}

void save_movies() {
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;

   /* Open database */
   rc = sqlite3_open("mc833.db", &db);
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }

   /* Create SQL statement */
   sql = "INSERT INTO MOVIE (ID,TITLE,LAUNCHDATE,GENRE,COPIES) "  \
         "VALUES (1, 'Paul', '04-02-2000', 'Horror', 2 ); " \
         "INSERT INTO MOVIE (ID,TITLE,LAUNCHDATE,GENRE,COPIES) "  \
         "VALUES (2, 'Paul4', '04-02-2000', 'Horror', 2 ); " \
         "INSERT INTO MOVIE (ID,TITLE,LAUNCHDATE,GENRE,COPIES) "  \
         "VALUES (3, 'Paul3', '04-02-2000', 'Horror', 2 ); " \
         "INSERT INTO MOVIE (ID,TITLE,LAUNCHDATE,GENRE,COPIES) "  \
         "VALUES (4, 'Paul2', '04-02-2000', 'Horror', 2 ); ";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Records created successfully\n");
   }
   sqlite3_close(db);
}