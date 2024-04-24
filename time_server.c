#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

void signalHanlder(int signo) {
    pid_t pid = wait(NULL);
    printf("Child process terminated, pid = %d\n", pid);
}


int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    signal(SIGCHLD, signalHanlder);

    while (1) {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);
        char* greet = "\nChoose your format(GET_TIME [format]), format can be:\ndd/mm/yyyy\ndd/mm/yy\nmm/dd/yyyy\nmm/dd/yy\n";
        send(client, greet, strlen(greet), 0);
        if (fork() == 0) {
            close(listener);
            char buf[256];
            while (1) {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;
                buf[ret] = 0;
                char cmd[32], format[32], tmp[32];
                int n = sscanf(buf, "%s %s %s", cmd, format, tmp);
                int error = 0;
                if ( n == 2 && strcmp("GET_TIME", cmd) == 0) {
                    error = 0;
                    printf("\nFormat: %s", format);
                    time_t current_time;
                    struct tm *time_info;
                    time(&current_time);
                    time_info = localtime(&current_time);
                    int day = time_info->tm_mday;
                    int month = time_info->tm_mon + 1;
                    int year = time_info->tm_year + 1900;
                    //formating(timeformat, day, month, year, format);
                    time_t now = time(0);
                    char sday[8];
                    char smonth[8];
                    char syear[8];
                    if (day < 10) sprintf(sday, "0%d", day); else sprintf(sday, "%d", day);
                    if (month < 10) sprintf(smonth, "0%d", month); else sprintf(smonth, "%d", month);
                    if (year < 2010) sprintf(syear, "0%d", year - 2000); else sprintf(syear, "%d", year - 2000);
                    char timeformat[256];
                    if (strcmp(format, "dd/mm/yyyy") == 0) {
                        sprintf(timeformat, "%s/%s/%d\n", sday, smonth, year);
                    }
                    else if (strcmp(format, "mm/dd/yyyy") == 0) {
                        sprintf(timeformat, "%s/%s/%d\n", smonth, sday, year);
                    } 
                    else if (strcmp(format, "dd/mm/yy") == 0) {
                        sprintf(timeformat, "%s/%s/%s\n", sday, smonth, syear);
                    } 
                    else if (strcmp(format, "mm/dd/yy") == 0) {
                        sprintf(timeformat, "%s/%s/%s\n", smonth, sday, syear);
                    }
                    else {
                        sprintf(timeformat, "Format not supported, please choose an existing one above!");
                    }
                    printf("\nTIME SENDED: %s", timeformat );
                    send(client, timeformat, strlen(timeformat), 0);
                } else {
                    error = 1;
                }
                if (error) {
                    char *msg = "Wrong format, please try again.\n";
                    send(client, msg, strlen(msg), 0);
                }
                printf("\nReceive buffer: %s", buf);
            }

            exit(0);
        }

        close(client);
    }

    return 0;
}