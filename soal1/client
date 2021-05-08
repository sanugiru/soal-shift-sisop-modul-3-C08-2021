#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#define PORT 8080
  
int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char reglog[100];
    char buffer[1024] = {0};
    char password[100], id[100];
    char message_client[100];
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    read(sock, buffer, strlen(buffer));
    scanf("%s", message_client);
    send(sock, message_client, strlen(message_client),0);
    if (strcmp(message_client, "register")==0){
        memset(buffer,0,strlen(buffer));
        read(sock, buffer, 1024);
        scanf("%s", id);
        send(sock, id, strlen(id), 0);
        memset(buffer,0,strlen(buffer));
        read(sock,buffer,strlen(buffer));
        scanf("%s", password);
        send(sock, password, strlen(password), 0);
        memset(buffer,0,strlen(buffer));
        read(sock,buffer,strlen(buffer));
    }
    else if (strcmp(message_client, "login")==0){
        memset(buffer,0,strlen(buffer));
        read(sock, buffer, 1024);
        scanf("%s", id);
        send(sock, id, strlen(id), 0);
        memset(buffer,0,strlen(buffer));
        read(sock,buffer,strlen(buffer));
        scanf("%s", password);
        send(sock, password, strlen(password), 0);
        memset(buffer,0,strlen(buffer));
        read(sock,buffer,strlen(buffer));
    }
}
