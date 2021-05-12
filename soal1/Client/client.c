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
#include <string.h>
#define PORT 8080

#define HANDSHAKE "800"
#define LOGIN "1"
#define REGISTER "2"
#define SUCCESS "3"
  
int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char reglog[100];
    char buffer[BUFSIZ];
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

    send(sock, HANDSHAKE, strlen(HANDSHAKE),0);
    fprintf(stdout, "Waiting connection\n");
    memset(buffer,0,sizeof(buffer));
    valread = read(sock,buffer,BUFSIZ);
    fprintf(stdout, "Connected\n");

    char username[100], password[100], authentication_data[100], registration_data[100];
    while (1){
        char temp[100];
        fprintf(stdout, "Choice login or register : ");
        fprintf(stdin, "%s", temp);
        if (strcmp(temp, "login")==0){
            send(sock, LOGIN, strlen(LOGIN),0);
            memset(buffer,0,sizeof(buffer));
            valread = read(sock, buffer, BUFSIZ);
            fprintf(stdout, "Login\nUsername : ");
            fprintf(stdin, "%s", username);
            fprintf(stdout, "Password : ");
            fprintf(stdin, "%s", password);
            sprintf(authentication_data, "%s:%s", username, password);

            send(sock, authentication_data, strlen(authentication_data), 0);
            memset(buffer,0,sizeof(buffer));
            valread = read(sock, buffer, BUFSIZ);

            if (strcmp(buffer, SUCCESS)==0){
                fprintf(stdout, "Add data successfully\n");
                // ----------authentication function
            }
            else fprintf(stdout, "Add data failed\n");
        }
        else if (strcmp(temp, "register")==0){
            send(sock, REGISTER, strlen(REGISTER), 0);
            
            fprintf(stdout, "Register\nUsername : ");
            fprintf(stdin, "%s", username);
            fprintf(stdout, "Password : ");
            fprintf(stdin, "%s", password);
            sprintf(registration_data, "%s:%s", username, password);

            send(sock, registration_data, strlen(registration_data), 0);
            memset(buffer, 0, sizeof(buffer));
            valread = read(sock, buffer, BUFSIZ);

            if (strcmp(buffer, SUCCESS)==0){
                fprintf(stdout, "Registration success\n");
            }
            else fprintf(stdout, "Registration failed\n");            
        }
        else fprintf(stdout, "Try again\n");
    }
}
