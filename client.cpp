#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <cstring>
#include <cctype>
#include <cstdio>
#include <string>
#define SIZE 2000
struct sockaddr_in server;
int port, sd;
char msg[SIZE];
int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("Sintax: %s <ip_address> <port>\n", argv[0]);
        return -1;
    }
    port = atoi(argv[2]);
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la socket().\n");
        return errno;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);
    if(connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }
    while(1)
    {
        printf("[client]$ ");
        fflush(stdout);
        bzero(&msg, sizeof(msg));
        int lungime;
        read(0, msg, sizeof(msg));
        lungime = strlen(msg);
        if(write(sd, &lungime, sizeof(lungime)) <= 0)
        {
            perror("[client]Eroare la write() catre server.\n");
            return errno;
        }
        if(write(sd, msg, lungime) <= 0)
        {
            perror("[client]Eroare la write() catre server.\n");
            return errno;
        }
        msg[strlen(msg) - 1] = '\0';
        if(!strcmp(msg, "quit"))
        {
            printf("Quitting server.\n");
            fflush(stdout);
            exit(0);
        }
        if(read(sd, &lungime, sizeof(int)) < 0)
        {
            perror("[client]Eroare la read() de la server.\n");
            return errno;
        }
        int totalCitit=0;
        bzero(&msg, sizeof(msg));
        while(lungime > totalCitit)
        {
            int citit = 0;
            char aux[SIZE];
            bzero(&aux, sizeof(aux));
            if((citit = read(sd, aux, lungime)) < 0)
            {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            totalCitit += citit;
            strcat(msg, aux);
        }

        if(msg[0] == 'h' && msg[1] == 't' && msg[2] == 't' && msg[3] == 'p')
        {
            char url[500];
            unsigned int j = 0;
            while(msg[j] != '\n')
            {
                url[j] = msg[j];
                j++;
            }
            url[j] = '\0';
            char aux[500];
            strcpy(aux, msg + j + 1);
            strcpy(msg, aux);
            j = 0;
            char nume_fisier[500];
            while(msg[j] != '\n')
            {
                nume_fisier[j] = msg[j];
                j++;
            }
            nume_fisier[j] = '\0';
            strcpy(aux, msg + j + 1);
            strcpy(msg, aux);
            CURL* curl;
            FILE* fp;
            CURLcode res;
            curl = curl_easy_init();
            if (curl)
            {
                fp = fopen(nume_fisier, "wb");
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                res = curl_easy_perform(curl); /*it performs the transfer as described in the options*/
                curl_easy_cleanup(curl); /*close all connections this handle has used*/
                fclose(fp);
            }
        }
        printf("[client] %s\n", msg);
        fflush(stdout);
    }
    close(sd);
    return 0;
}