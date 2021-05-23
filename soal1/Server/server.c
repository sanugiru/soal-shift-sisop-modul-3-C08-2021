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
void upload_request(int sock, char filename[]);
int exist_file(char file_path[]);
void check_tsv(char string[], char flag[], char delimiter, int *index);
int substring_tsv(char temp[], char filename[]);
int substring(char line[], char filename[]);
void handle_delete(char file_name[], int line_number);
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

                        fprintf(stdout, "Enter handle add request\n");

                        send(sock, SUCCESS, strlen(SUCCESS), 0);
                        fprintf(stdout, "Sent status %s\n", SUCCESS);

                        FILE *fp;
                        fp = fopen("file.tsv", "a+");
                        if (fp == NULL) {
                            perror("open file failed");
                            exit(EXIT_FAILURE);
                        }
                        
                        memset(buffer,0,sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);

                        char filename[50];
                        char publisher[50];
                        char tahun_pub[10];
                        int index =0;
                        check_tsv(buffer, filename, ':', &index);
                        check_tsv(buffer, publisher, ':', &index);
                        check_tsv(buffer, tahun_pub, ':', &index);

                        char file_path[100];
                        sprintf(file_path, "FILES/%s", filename);

                        upload_request(sock, filename);

                        fprintf(fp, "%s\t", file_path);
                        fprintf(fp, "%s\t", publisher);
                        fprintf(fp, "%s\n", tahun_pub);
                        
                        send(sock, SUCCESS, strlen(SUCCESS), 0);

                        FILE *fp_log;
                        fp_log = fopen("running.log", "a+");

                        fprintf(fp_log, "Tambah: %s (%s)\n", filename, request);

                        fclose(fp_log);
                        fclose(fp);
                    }
                    else if (strcmp(request, DOWNLOAD)==0){
                        int fd;
                        int sent_bytes = 0;
                        char file_size[256];
                        struct stat file_stat;
                        off_t offset;
                        int remain_data;

                        send(sock, SUCCESS, strlen(SUCCESS), 0);

                        memset(buffer,0,sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);

                        char filename[50];
                        strcpy(filename, buffer);

                        char file_path[100];
                        sprintf(file_path, "/home/deka/modul3/Server/FILES/%s", filename);

                        fprintf(stdout, "%s %ld\n", file_path, strlen(file_path));

                        if (exist_file(file_path)) {
                            send(sock, SUCCESS, strlen(SUCCESS), 0);
                            
                            memset(buffer,0,sizeof(buffer));
                            valread = read(sock, buffer, BUFSIZ);

                            fd = open(file_path, O_RDONLY);

                            if (fstat(fd, &file_stat) < 0) {
                                    perror("filestat error");
                                    exit(EXIT_FAILURE);
                            }

                            sprintf(file_size, "%ld", file_stat.st_size);
                            send(sock, file_size, sizeof(file_size), 0);

                            offset = 0;
                            remain_data = file_stat.st_size;

                            while (((sent_bytes = sendfile(sock, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0)) {   
                                    remain_data -= sent_bytes;
                            }
                        } else {
                            send(sock, FAIL, strlen(FAIL), 0);
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
                        send(sock, SUCCESS, strlen(SUCCESS), 0);
                        memset(buffer,0,sizeof(buffer));
                        valread = read(sock, buffer, BUFSIZ);

                        char filename_to_delete[50];
                        strcpy(filename_to_delete, buffer);

                        char line[256];
                        char formatted_data[BUFSIZ];
                        
                        char filename[] = "file.tsv";
                        FILE *fp = fopen(filename, "r");
                        int line_counter = 0;

                        formatted_data[0] = '\0';

                        int is_file_exist = 0;

                        while (fgets(line, sizeof line, fp)) {
                            line_counter++;
                            if (substring(line, filename_to_delete)){
                                char file_path[100];
                                sprintf(file_path, "FILES/%s", filename_to_delete);

                                char file_path_new[100];
                                sprintf(file_path_new, "FILES/old-%s", filename_to_delete);

                                handle_delete(filename, line_counter);

                                rename(file_path, file_path_new);
                                is_file_exist = 1;
                                break;      
                            }
                        }

                        FILE *fp_log;
                        fp_log = fopen("running.log", "a+");

                        if (is_file_exist) {
                            fprintf(fp_log, "Hapus: %s (%s)\n", filename_to_delete, request);

                            send(sock, SUCCESS, strlen(SUCCESS), 0);
                        } else {
                            send(sock, FAIL, strlen(FAIL), 0);
                        }

                        fclose(fp);
                        fclose(fp_log);
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

void upload_request(int sock, char filename[]) {
    char buffer[BUFSIZ];
    int valread;

    int file_size;
    int remain_data;
    ssize_t len;
    FILE *received_file;

    send(sock, SUCCESS, strlen(SUCCESS), 0);

    memset(buffer,0,sizeof(buffer));
    recv(sock, buffer, BUFSIZ, 0);
    file_size = atoi(buffer);

    fprintf(stdout, "Received file with size %d bytes\n", file_size);

    char file_path[100];
    sprintf(file_path, "FILES/%s", filename);

    received_file = fopen(file_path, "w");
    if (received_file == NULL) {
            perror("Failed to open file");
            exit(EXIT_FAILURE);
    }

    remain_data = file_size;

    while ((remain_data > 0) && ((len = recv(sock, buffer, BUFSIZ, 0)) > 0)) {
            fwrite(buffer, sizeof(char), len, received_file);
            remain_data -= len;
            fprintf(stdout, "Receive %ld bytes and %d bytes remaining\n", len, remain_data);
    }
    fclose(received_file);
}

int substring(char line[], char filename[]) {
    char filename_exist[50];
    char delimiter = '\t';
    int idx = 0;
    check_tsv(line, filename_exist, delimiter, &idx);
    if (strstr(filename_exist, filename))
        return 1;
    else 
        return 0;
}

void handle_delete(char file_name[], int line_number) {
    FILE *fp, *fp_temp;
    char str[BUFSIZ];
    char file_name_temp[] = "temp_file.txt";
    int counter = 0;
    char ch;

    fp = fopen(file_name, "r");
    fp_temp = fopen(file_name_temp, "w"); 

    while (!feof(fp)) {
        strcpy(str, "\0");
        fgets(str, BUFSIZ, fp);
        if (!feof(fp)) {
            counter++;
            if (counter != line_number) {
                fprintf(fp_temp, "%s", str);
            }
        }
    }
    fclose(fp);
    fclose(fp_temp);
    remove(file_name);
    rename(file_name_temp, file_name);
}
