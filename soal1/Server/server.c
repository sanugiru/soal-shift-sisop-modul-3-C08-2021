#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#define PORT 8080

int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *message = "register or login\n";
    char password[100];
    char id[100];

    DIR* id_pass_txt = opendir("files");
    if (ENOENT == errno) mkdir("files",0777);
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // register or login
    send(new_socket, message, strlen(message), 0);
    read(new_socket, buffer, 1024);   

    FILE* akun = fopen("files/akun.txt", "a+");
    FILE* databases = fopen("files/files.tsv", "a+");
    memset(password,0,sizeof(password));
    memset(id,0,sizeof(id));
    if (strcmp(buffer, "register")==0){
        message = "id : ";
        send(new_socket, message, strlen(message), 0);
        read(new_socket, id, 100);
        message = "password : ";
        send(new_socket, message, strlen(message), 0);
        read(new_socket, password, 100);
        fprintf(akun, "%s:%s\n", id, password);
        message = "registration success\n";
        send (new_socket, message, strlen(message),0);
    }
    else if(strcmp(buffer,"login")==0){
        char clients[100], check_password[100];
        int flag = 0;
        read(new_socket, id, 100);
        read(new_socket, password, 100);
        sprintf(clients, "%s:%s", id, password);
        while (fscanf(akun, "%s", check_password)==1){
            if (strstr(clients, clients)) {
                message = "login success\n";
                flag = 1;
                send(new_socket, message, strlen(message), 0);
                break;
            }
        }
        if (flag == 0) {
            message = "sign in first\n";
            send(new_socket, message, strlen(message), 0);
        }
        
    }
    fclose(akun);
    fclose(databases);

    return 0;
}
