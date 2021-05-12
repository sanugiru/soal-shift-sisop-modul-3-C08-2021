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
#include <fcntl.h>
#include <sys/sendfile.h>
#define PORT 8080

#define CONNECT "800"
#define LOGIN "1"
#define REGISTER "2"
#define SUCCESS "3"
#define ADD "4"
#define DOWNLOAD "5"
#define SEE "6"
#define DELETE "7"
#define FIND "8"

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

    send(sock, CONNECT, strlen(CONNECT),0);
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
                char size[100], filename[100];
                int remain, file_size;
                while(1){
                    fprintf(stdout, "choice one : add, download, delete, see, or find");
                    char temp2[100];
                    fscanf(stdin, "%s", temp2);
                    if (strcmp(temp2, "add")){
                        char publish[100], year[10], file_path[100], add[100];
                        int flag, sent=0;
                        // sent = sent bytes
                        // remain = remain data
                        // add = add data
                        struct stat file_stat;
                        off_t offset = 0;
                        send(sock, ADD, strlen(ADD), 0);
                        memset(buffer,0,sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);
                        fprintf(stdout, "Publisher Username : ");
                        fprintf(stdin, "%s", publish);
                        fprintf(stdout, "Tahun Publikasi : ");
                        fprintf(stdin, "%s", year);
                        fprintf(stdout, "Filepath: ");
                        fprintf(stdin, "%s", file_path);
                        sprintf(add, "%s:%s:%s", publish, year, file_path);
                        
                        // upload request
                        send(sock, add, strlen(add), 0);
                        memset(buffer,0,sizeof(buffer));
                        valread = read(sock,buffer,BUFSIZ);
                        flag = open(file_path, O_RDONLY);
                        
                        if(fstat(flag, &file_stat)<0){
                            perror("filestat error");
                            exit(EXIT_FAILURE);
                        }
                        sprintf(size, "%ld", file_stat.st_size);
                        send(sock, size, strlen(size), 0);
                        remain = file_stat.st_size;

                        while(((sent = sendfile(sock, flag, &offset, BUFSIZ))>0) && (remain > 0)){
                            remain = remain - sent;
                        }
                        memset(buffer, 0, sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);
                        if (strcmp(buffer, SUCCESS)==0){
                            fprintf(stdout, "Add data is success\n");
                        }
                        else fprintf(stdout, "Add data is failed\n");
                    }
                    else if (strcmp(temp2, "download")){
                        ssize_t flag_download;
                        send(sock, DOWNLOAD, strlen(DOWNLOAD), 0);
                        memset(buffer, 0 , sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);
                        
                        fscanf(stdin, "%s", filename);
                        send(sock, filename, strlen(filename), 0);
                        memset(buffer, 0, sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);
                        if (strcmp(buffer, SUCCESS)==0){
                            FILE *file_receive;
                            send(sock, SUCCESS, strlen(SUCCESS), 0);
                            memset(buffer, 0,sizeof(buffer));
                            recv(sock, buffer, BUFSIZ, 0);
                            file_size = atoi(buffer);
                            // atoi = mengubah alphabet ke bilangan bulat
                            file_receive = fopen(filename, "w");
                            if (file_receive == NULL){ 
                                perror("Failed to open file");
                                exit(EXIT_FAILURE);
                            }
                            remain = file_size;
                            while ((remain>0) && ((flag_download = recv(sock, buffer, BUFSIZ, 0))>0)){
                                fwrite(buffer, sizeof(char), flag_download, file_receive);
                                remain = remain - flag_download;
                            }
                            fclose(file_receive);
                        }
                        else fprintf(stdout, "File %s doesn't exist", filename);
                    }
                    else if (strcmp(temp2, "delete")){
                        send(sock, DELETE, strlen(DELETE), 0);
                        memset(buffer, 0, sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);
                        fscanf(stdin, "%s", filename);
                        send (sock, filename, strlen(filename), 0);
                        memset(buffer,0,sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);

                        if (strcmp(buffer, SUCCESS)==0){
                            fprintf(stdout, "File %s deleted\n", filename);
                        }
                        else fprintf(stdout, "File %s doesn't exist\n", filename);
                    }
                    else if (strcmp(temp2, "see")){
                        send(sock, SEE, strlen(SEE), 0);
                        memset(buffer, 0, sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);
                        fprintf (stdout, "%s", buffer);
                    }
                    else if (strcmp(temp2, "find")){
                        send(sock, FIND, strlen(FIND), 0);
                        memset(buffer,0,sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);
                        fscanf(stdin, "%s", filename);
                        send(sock, filename, strlen(filename), 0);
                        memset(buffer, 0, sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);
                        if (strlen(buffer) > 0){
                            fprintf (stdout, "%s", buffer);
                        }
                        else fprintf(stdout, "File with substring %s doesn't exist\n", filename);
                    }
                    else fprintf(stdout, "Command is not valid\n");
                }
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
        else fprintf(stdout, "Try a bit harder next time\n");
    }
}
