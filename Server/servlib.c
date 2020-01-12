#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <signal.h>
#include "servlib.h"
#include <time.h>
#include <math.h>
#include <fcntl.h>

char* logfile;
time_t timeStartListen;
int numberServed = 0;
int numberSuccess = 0;
int daemonMode = 0;
int modeServer;

void modeDaemon() {
    // redirect stdin, stdout, and stderr to /dev/NULL
    (void) close(0);
    (void) close(1);
    (void) close(2);
    if (open("/dev/null", O_RDWR) != 0) {
        show(messError(errno));
        exit(EXIT_FAILURE);
    }

    // Our process ID and Session ID
    pid_t pid, sid;

    // Fork off the parent process
    pid = fork();
    if (pid < 0) {
        show(messError(errno));
        exit(1);
    } else if (pid > 0) { // child can continue to run even after the parent has finished executing
        exit(EXIT_FAILURE);
    }
    // create new session
        sid = setsid();
        if (sid < 0) {
            exit(EXIT_FAILURE);
        }
    pid = fork();
    if (pid < 0) {
        show(messError(errno));
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // child can continue to run even after the parent has finished executing
        exit(EXIT_FAILURE);
    }

    daemonMode = 1;

    //reopen stdout, stderr
    char* filename = logfile != NULL? logfile : "/tmp/lab2.log";
    stderr=fopen(filename,"w");
    stdout=fopen(filename,"w");
    stdin=fopen(filename,"w");

    umask(0);

}

// Set time, when server starts to listen
void timeListen() {
	timeStartListen = time(NULL);
}

// Get format of the current time
char* currentTime(time_t time) {
	char* time_h_m_s = (char*) calloc(300, sizeof(char));
	struct tm *timeinfo = localtime(&time);
	sprintf(time_h_m_s, "%02d.%02d.%04d %02d:%02d:%02d ", 
				timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year + 1900,
				timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	return time_h_m_s;
}

// Add time to message
char* addTime(char* message) {
	char* timeInString = currentTime(time(NULL));
	return strcat(timeInString, message);
}

// Add a new string to the file
void writeToFile(char* filename, char* content){
	FILE *fp = fopen(filename, "a+");
	if (fp == NULL )perror("Error: \n");
	write(fileno(fp), content, strlen(content));
	fclose(fp);
}

// Output messages to the logfile and the standard stream
void show(char* message) {
	if (daemonMode == 0)	
		write(1,message,strlen(message));
	if (logfile != NULL) {
		char* messTime = addTime(message);
		writeToFile(logfile, messTime);
	}
}

char* messError(int errornum) {
	char* message = (char*) calloc(100, sizeof(char));
	sprintf(message, "ERROR %d: %s\n", errornum, hstrerror(errornum));
	return message;
};

// Create message of catch request from client
char* request(char* request) {
	char* message = (char *) calloc(100, sizeof(char));
	sprintf(message, "Required request: %s\n", request);
	return message;
}

char *reverse(char *str){

    char* rever = (char*) calloc(255, sizeof(char));
    int i, j, len;

    j = 0;
    len = strlen(str);

    for (i = len - 1; i >= 0; i--)
    {
        rever[j++] = str[i];
    }
    return rever;
}
int checkNumber(char* numberString) {
	int dot=0;
	if (strlen(numberString) == 0) return 0;
	if(numberString[0]=='_'){
		for (int i = 1; i < strlen(numberString); i++) {
			if(numberString[i]=='.')
				++dot;
			if ((numberString[i] > '9' || numberString [i] < '0') && (numberString[i] != '.'))
				return 0;		
		}
		if(dot>1) return 0;
		return 1;
	
	}
	if (numberString[0] > '9' || numberString [0] < '0') return 0;
	
	
	for (int i = 1; i < strlen(numberString); i++) {
		if(numberString[i]=='.')
			++dot;
		if ((numberString[i] > '9' || numberString [i] < '0') && (numberString[i] != '.'))
			return 0;
	}
	if(dot>1) return 0;
	return 1;
}

int isValidPutRequest (char* requestString) {
	char* word = (char*) calloc(16, sizeof(char));
	char* number = (char*) calloc(16, sizeof(char));
	sscanf(requestString, "%s %s", word, number);
	if (strcmp(word, "PUT") != 0) return 0;
	if (checkNumber(number) == 0) return 0;
	return 1;
}

request_t typeRequest(char* requestString) {
	char* word = (char*) calloc(16, sizeof(char));
	sscanf(requestString, "%s", word);
	if (strcmp(requestString, "GET") == 0) return GET;
	if (isValidPutRequest(requestString) == 1) return PUT;
	return -1;
}

// Get number from request "PUT Number" 
char*getNumFormPut(char* request) {
	char* number = (char*) calloc(16, sizeof(char));
	sscanf(request, "PUT %s", number);
	return number;
}
int indexOfFinalSpace(char* str) {
    for (int i = strlen(str) - 2; i >= 0; i--) {
        if (str[i] == ' ') return i;
    }
    return -1;
}

char* getRemaind(char* str, int index) {
    char* remainNumbers = (char*) calloc(255, sizeof(char));
    memcpy(remainNumbers, str, index + 1);
    return remainNumbers;
}


char* getFinal(char* str, int index) {
    char* remainNumbers = (char*) calloc(255, sizeof(char));
    memcpy(remainNumbers, str + index + 1, strlen(str));
    if(strlen(remainNumbers)==1)
    return remainNumbers;
    else return reverse(remainNumbers);
}

// Rewrite file data
void rewriteData(char* filename, char* content) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        show(messError(errno));
        exit(EXIT_FAILURE);
    }
    fputs(content, fp);
    fclose(fp);
}

// Get list number from file in database
char* getNumFormDatabase(char* filename) {
    char* remainNumbers;
	char* messResponse = (char*) calloc(255, sizeof(char));
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		messResponse = "Empty!!!";
	} else {
        char* allNumbers = (char*) calloc(255, sizeof(char));
        fgets(allNumbers, 255, fp);
        if (allNumbers[0] == '\0') {
            messResponse = "Not any number";
        } else {
            int seperator = indexOfFinalSpace(reverse(allNumbers));
            remainNumbers = getRemaind(reverse(allNumbers), seperator);
            rewriteData(filename,reverse(remainNumbers) );
            messResponse = getFinal(reverse(allNumbers), seperator);
        }
	}
	return messResponse;
}

// Get file data of IP client
char* path(char* clientIP) {
    char* pathfile = (char*) calloc(255, sizeof(char));
    if (modeServer == PRIVATE) {
        sprintf(pathfile, "Database/%d/%s", getppid(), clientIP);
    } else {
        sprintf(pathfile, "Database/%d/public", getppid());
    }
    return pathfile;
}

// Handle request
char* handleRequest(char* request, char* fromClientIP) {
	char* messResponse = (char*) calloc(255, sizeof(char));
	char* pathfile = path(fromClientIP);
	request_t type = typeRequest(request);
	switch(type) {
		case PUT:
			writeToFile(pathfile,getNumFormPut(request));
			writeToFile(pathfile, " ");
			messResponse = "OK";
			break;
		case GET:
			messResponse = getNumFormDatabase(pathfile);
			break;
		default:
			messResponse = "ERROR!!!, please try again!";
	}
	return messResponse;
}

// Create message of response to client
char* response(char* request) {
	char* message = (char *) calloc(100, sizeof(char));
	sprintf(message, "Response to request: %s\n", addTime(request));
	kill(getppid(), SIGUSR2);
	return message;
}


// Print help
void help() {
	printf("Usage: ./server -p[PORT] -w[seconds] -d -l[path] -a[IP] -v -h \n");
	printf("\n");
	printf("  -w\t Simulate operation by pausing the process / thread serving the request for N seconds. "
              "If the option is not set, serve the request without delay.default: 0).\n");
	printf("  -d\t Work in demon mode.\n");
	printf("  -l\t path to logfile (default: /tmp/lab2.log).\n");
	printf("  -a\t The address on which the server listens and to which the client connects.\n");
	printf("  -p\t The port on which the server listens and to which the client connects.\n");
	printf("  -v\t version\n");
	printf("  -h\t help\n");
}

// Format time hh:mm:ss
char* showTime(int second) {
	char* hh_mm_ss = (char*) calloc(10, sizeof(char));
	int hour = floor(second / 3600);
	int min = floor((second % 3600) / 60);
	int sec = second - 3600 * hour - 60 * min;
	sprintf(hh_mm_ss, "%02dh%02dm%02ds", hour, min, sec);
	return hh_mm_ss;
}

// Create message of working time
char* workingTime() {
	char* message = (char*) calloc(100, sizeof(char));
	float workingSeconds = difftime(time(NULL), timeStartListen);
	sprintf(message, "Working time of server: %s \n", showTime(workingSeconds));
	return message;
}

// Create message of counting required request
char* CountHost() {
	char* message = (char*) calloc(100, sizeof(char));
	sprintf(message, "Number of successful services: %d \nNumber of failed attempts: %d\n",
			numberSuccess,numberServed - numberSuccess);
	return message;
}

// Handler signal
void signalHandler(int sig) {
	switch(sig) {
		case SIGINT:
			show("Received SIGINT\n");
			_exit(0);
		case SIGQUIT:
			show("Received SIGQUIT\n");
			_exit(0);
		case SIGTERM:
			show("Received SIGTERM\n");
			_exit(0);
		case SIGUSR1:
			show(workingTime());
			show(CountHost());
			break;
		case SIGUSR2:
			numberSuccess++;
	}
}

void ignoreSIGCHLD() {
	struct sigaction act;
	bzero(&act, sizeof(act));
	act.sa_handler = SIG_IGN;
	act.sa_flags = SA_RESTART;
	sigaction(SIGCHLD, &act, NULL);
}

// Create folder to save data of server
void database(int mode) {
    char* directory = (char*) calloc(255, sizeof(char));
    sprintf(directory, "Database/%d", getpid());
    mkdir("Database", 0777);
    mkdir(directory, 0777);
    modeServer = mode;
}

// Remove database
void deleteDatabase() {
	char* cmd = (char*) calloc(255, sizeof(char));
	sprintf(cmd, "rm -rf Database/%d", getpid());
	system(cmd);
}

// increase number served request
void increaseNumberServed(){
	numberServed++;
}

// Create logfile and save name of logfile to logfilename.txt
void createLogfile(char* logfileName) {
	FILE* fp = fopen(logfileName, "w");
	if (fp == NULL) {
		printf("ERROR %d: %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	fclose(fp);
	logfile = logfileName;
}



