#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <pthread.h>
#include <wait.h>

#include <time.h>
#define PORT 8080
#define LOGIN "2"
#define REGISTER "3"
#define SUCCESS "4"
#define ADD "5"
#define DOWNLOAD "6"
#define SEE "7"
#define DELETE "8"
#define FIND "9"
#define FAIL "10"

// socketdfd = newsocket

char buffer[BUFSIZ];
int server_fd, sock, valread;

void *new_connection();
int exist_file(char file_path[]);
void check_tsv(char string[], char flag[], char delimiter, int *index);
int substring_tsv(char temp[], char filename[]);
char* login();

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int opt = 1, connection = 0;
    int addrlen = sizeof(address);
      
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
    mkdir("FILES", 0700);
    pthread_t thread_id[100];
    while (1){
        if ((sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pthread_create(&(thread_id[connection]), NULL, &new_connection, &sock);
        pthread_join(thread_id[connection], NULL);
        connection++;
    }
    return 0;
}

void* new_connection(){
    memset(buffer,0,sizeof(buffer));
    valread= read(sock,buffer,BUFSIZ);
    fprintf(stdout, "Connection received from %s\n", buffer);
    send(sock, SUCCESS, strlen(SUCCESS), 0);
    fprintf(stdout, "Send Response %s\n", SUCCESS);

    while(1){
        char auth[100];
        memset(buffer, 0, sizeof(buffer));
        valread = read(sock, buffer, BUFSIZ);
        send(sock, SUCCESS, strlen(SUCCESS), 0);
        if (strcmp(buffer, LOGIN)==0){
            char credential[1024];
            FILE *file = fopen("/home/deka/modul3/Server/akun.txt", "r");
            int flag_login = 0;
            memset(buffer, 0, sizeof(buffer));
            valread = read(sock, buffer, BUFSIZ);
            while (fscanf(file, "%s", credential) != EOF){
                if(strcmp(buffer, credential)== 0){
                    flag_login = 1;
                    break;
                }
            }
            fclose(file);
            if (flag_login == 1){
                send (sock, SUCCESS, strlen(SUCCESS), 0);
                strcpy(auth, buffer);
                while(1){
                    char request[50];
                    memset(buffer, 0, sizeof(buffer));
                    valread = read(sock, buffer, BUFSIZ);
                    strcpy(request, buffer);
                    if(strcmp(request, ADD)==0){
                        char filename[100], publisher[100], tahun_publikasi[10], filepath[100];
                        int size, remain;
                        ssize_t len;
                        send(sock, SUCCESS, strlen(SUCCESS),0);
                        FILE *file_add;
                        file_add = fopen("/home/deka/modul3/Server/file.tsv", "a+");
                        memset(buffer,0,sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);
                        // extract
                        int temp_add=0;
                        check_tsv(buffer,filename, ':', &temp_add);
                        check_tsv(buffer,publisher, ':', &temp_add);
                        check_tsv(buffer,tahun_publikasi, ':', &temp_add);
                        sprintf(filepath, "FILES/%s", filename);
                        file_add = fopen(filepath, "w");
                        //
                        send(sock, SUCCESS, strlen(SUCCESS), 0);
                        memset(buffer,0,sizeof(buffer));
                        recv(sock,buffer,BUFSIZ,0);
                        size = atoi(buffer);

                        if (file_add == NULL){
                            perror("File doesn't exist\n");
                            exit(EXIT_FAILURE);
                        }
                        remain = size;
                        while((remain>0)&&((len=recv(sock,buffer,BUFSIZ,0))>0)){
                            fwrite(buffer, sizeof(char), len, file_add);
                            remain -= len;
                            fprintf(stdout, "Receive %ld bytes and remain %d bytes", len, remain);
                        }
                        fclose(file_add);

                        fprintf(file_add, "%s\t", filepath);
                        fprintf(file_add, "%s\t", publisher);
                        fprintf(file_add, "%s\n", tahun_publikasi);
                        send(sock, SUCCESS, strlen(SUCCESS), 0);
                        FILE *log_file = fopen("running.log", "a+");
                        
                        fprintf(log_file, "Tambah: %s (%s)\n", filename, request);
                        fclose(log_file);
                        fclose(file_add);
                    }
                    else if (strcmp(request, DOWNLOAD)==0){
                        send(sock, SUCCESS, strlen(SUCCESS),0);
                        int remain, sent_bytes=0;
                        char size[100], file_path[100];
                        struct stat file_stat;
                        off_t offset;
                        int flag;
                        memset(buffer,0,sizeof(buffer));
                        valread = read(sock,buffer,BUFSIZ);
                        char file_download[100];
                        strcpy(file_download,buffer);

                        sprintf(file_path, "FILES/%s", file_download);
                        if(exist_file(file_path)){
                            send(sock,SUCCESS,strlen(SUCCESS),0);
                            memset(buffer,0,sizeof(buffer));
                            valread = read(sock,buffer,BUFSIZ);
                            flag = open(file_path, O_RDONLY);

                            if (fstat(flag, &file_stat)<0){
                                perror("File stat error");
                                exit(EXIT_FAILURE);
                            }
                            sprintf(size, "%ld", file_stat.st_size);
                            send(sock, size, sizeof(size),0);
                            fprintf(stdout, "Sen %s bytes\n", size);
                            offset = 0;
                            remain = file_stat.st_size;
                            while(((sent_bytes=sendfile(sock,flag,&offset,BUFSIZ))>0)&&(remain>0)){
                                remain -=sent_bytes;
                            }
                        }
                        else {
                            send(sock, FAIL, strlen(FAIL),0);
                        }
                    }
                    else if (strcmp(request, SEE)==0){
                        FILE *file_see;
                        file_see = fopen("file.tsv", "r");
                        char temp_see[1000], format[1024];
                        while (fgets(temp_see, sizeof temp_see, file_see)){
                            char name[100], publisher[100], tahun_publikasi[10];
                            char extension[100], filepath[100], folderpath[100];
                            int temp_see_tsv = 0;
                            fprintf(stdout, "folder path %s\n", folderpath);
                            check_tsv(temp_see,folderpath,'/',&temp_see_tsv);
                            check_tsv(temp_see,name,'.',&temp_see_tsv);
                            check_tsv(temp_see,extension,'\t',&temp_see_tsv);
                            sprintf(filepath, "%s/%s.%s", folderpath, name, extension);
                            check_tsv(temp_see,publisher,'\t',&temp_see_tsv);
                            check_tsv(temp_see,tahun_publikasi,'\n',&temp_see_tsv);
                            sprintf(format, "Nama: %s\nPublisher: %s\nTahun Publikasi: %s\nEkstensi file: %s\nFilepath: %s\n", name, publisher, tahun_publikasi, extension, filepath);
                        }
                        send(sock, format, strlen(format), 0);
                        fclose (file_see);
                    }
                    else if (strcmp(request, DELETE)==0){
                        char delete_file[100], format[1024], temp[500];
                        char filename[100];
                        memset(format,0,sizeof(format));
                        int temp_counter=0, temp_exist=0;
                        send(sock, SUCCESS, strlen(SUCCESS),0);
                        memset(buffer,0,sizeof(buffer));
                        valread=read(sock,buffer,BUFSIZ);
                        strcpy(delete_file,buffer);
                        char filename_delete[] = "file.tsv";
                        FILE *file_delete = fopen(filename_delete, "r");
                        
                        while(fgets(temp, sizeof temp, file_delete)){
                            temp_counter++;char delete_file[100], format[1024], temp[500];
                        char filename[100];
                        memset(format,0,sizeof(format));
                        int temp_counter=0, temp_exist=0;
                        send(sock, SUCCESS, strlen(SUCCESS),0);
                        memset(buffer,0,sizeof(buffer));
                        valread=read(sock,buffer,BUFSIZ);
                        strcpy(delete_file,buffer);
                        char filename_delete[] = "file.tsv";
                        FILE *file_delete = fopen(filename_delete, "r");
                        
                        while(fgets(temp, sizeof temp, file_delete)){
                            temp_counter++;
                            if(substring_tsv(temp,delete_file)){
                                char filepath_find[100];
                                // char new_filepath[100];
                                sprintf(filepath_find, "FILES/%s", delete_file);
                                // sprintf(new_filepath, "FILES/old-%s", delete_file);
                                remove(filename_delete);
                                // FILE *name_file_delete = fopen(file_delete,"r");
                            }
                        }
                            if(substring_tsv(temp,delete_file)){
                                char filepath_find[100];
                                // char new_filepath[100];
                                sprintf(filepath_find, "FILES/%s", delete_file);
                                // sprintf(new_filepath, "FILES/old-%s", delete_file);
                                remove(filename_delete);
                                // FILE *name_file_delete = fopen(file_delete,"r");
                            }
                        }
                    }
                    else if (strcmp(request, FIND)==0){
                        char filename[100], format[1024], temp_find[500];
                        FILE *file_find = fopen("/home/deka/modul3/Server/file.tsv","r");
                        send(sock,SUCCESS,strlen(SUCCESS),0);
                        memset(buffer,0,sizeof(buffer));
                        valread=read(sock,buffer,BUFSIZ);
                        strcpy(filename,buffer);
                        memset(format,0,sizeof(format));

                        while(fgets(temp_find, sizeof temp_find, file_find)!=NULL){
                            if(substring_tsv(temp_find, filename)){
                                char name[100], publisher[100], tahun_publikasi[10];
                                char extension[100], filepath[100], folderpath[100];
                                int temp_see_tsv = 0;
                                check_tsv(temp_find,folderpath,'/',&temp_see_tsv);
                                check_tsv(temp_find,name,'.',&temp_see_tsv);
                                check_tsv(temp_find,extension,'\t',&temp_see_tsv);
                                sprintf(filepath, "%s/%s.%s", folderpath, name, extension);
                                check_tsv(temp_find,publisher,'\t',&temp_see_tsv);
                                check_tsv(temp_find,tahun_publikasi,'\n',&temp_see_tsv);
                                sprintf(format, "Nama: %s\nPublisher: %s\nTahun Publikasi: %s\nEkstensi file: %s\nFilepath: %s\n", 
                                name, publisher, tahun_publikasi, extension, filepath);
                            }
                        }
                        send (sock, format, strlen(format), 0);
                        fclose(file_find);
                    }
                }
            }
            else send(sock, FAIL, strlen(FAIL), 0);
        }
        else if (strcmp(buffer, REGISTER)==0){
            FILE *file;
            file = fopen("/home/deka/modul3/Server/akun.txt", "a+");
            memset(buffer,0,sizeof(buffer));
            valread = read(sock, buffer, BUFSIZ);
            fprintf(file, "%s\n", buffer);
            send(sock, SUCCESS, strlen(SUCCESS), 0);
            fclose(file);
        }   
        else {
            perror("bad request");
            exit(0);
        }
    }
}


int substring_tsv(char temp[], char filename[]){
    char filename_subs[100];
    char delimiter = '\t';
    int index = 0;
    check_tsv(temp, filename_subs, delimiter, &index);
    if(strstr(filename_subs,filename)){
        return 1;
    }
    else return 0;
}

int exist_file(char file_path[]){
    char temp[100], path_exist[100];
    int temp_file_exist = 0;
    char filename[] = "file.tsv";
    FILE *file = fopen(filename, "r");
    while(fgets(temp, sizeof temp, file)){
        int flag = 0;
        check_tsv(temp, file_path, '\t', &flag);
        if (strcmp(path_exist,file_path)==0){
            temp_file_exist =1;
            break;
        }
    }
    if (temp_file_exist==1) return 1;
    else return 0;
}

void check_tsv(char string[], char flag[], char delimiter, int *index){
    int temp = 0;
    while (*index < strlen(string) && string[*index]!=delimiter){
        flag[temp] = string[*index];
        *index += 1;
        temp++;
    }
    flag[temp] = '\0';
    *index +=1;
}
