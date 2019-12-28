#ifndef HEADER_FILE
#define HEADER_FILE
#define PUBLIC 0
#define PRIVATE 1

enum request { PUT = 1, GET = 0, INVALID = -1 };
typedef enum request request_t;
void show(char*);
void help();
void database();
void createLogfile(char*);
char* messError(int);
void timeListen();
char* response(char*);
void signalHandler(int);
char* request(char*);
void modeDaemon();
char* handleRequest(char*, char*);
void deleteDatabase();
void increaseNumberServed();
void ignoreSIGCHLD();

#endif
