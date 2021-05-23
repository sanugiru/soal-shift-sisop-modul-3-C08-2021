# Soal Shift Sisop Modul 3 C08 2021  

Anggota:
- 05111940000112 Deka Julian Arrizki	    
- 05111940000141 Muhammad Farhan Haykal	
- 05111940000158 Shahnaaz Anisa Firdaus


## Daftar Isi
- [SOAL 1](#soal-1)
  * [**Pembahasan**](#pembahasan)
  * [**Kendala**](#kendala)
- [SOAL 2](#soal-2)
  * [**Pembahasan**](#pembahasan-1)
  * [**Kendala**](#kendala-1)
- [SOAL 3](#soal-3)
  * [**Pembahasan**](#pembahasan-2)
  * [**Kendala**](#kendala-2)


## SOAL 1
### **Pembahasan**
a. Pada saat client tersambung dengan server, terdapat dua pilihan pertama, yaitu register dan login. Jika memilih register, client akan diminta input id dan passwordnya untuk dikirimkan ke server. User juga dapat melakukan login. Login berhasil jika id dan password yang dikirim dari aplikasi client sesuai dengan list akun yang ada didalam aplikasi server. Sistem ini juga dapat menerima multi-connections. Koneksi terhitung ketika aplikasi client tersambung dengan server. Jika terdapat 2 koneksi atau lebih maka harus menunggu sampai client pertama keluar untuk bisa melakukan login dan mengakses aplikasinya. Keverk menginginkan lokasi penyimpanan id dan password pada file bernama akun.txt dengan format :

```
while (1){
        char temp[100];
        memset(temp,0,sizeof(temp));
        fprintf(stdout, "Choice login or register : ");
        fscanf(stdin, "%s", temp);
        if (strcmp(temp, "login")==0){
            send(sock, LOGIN, strlen(LOGIN),0);
            memset(buffer,0,sizeof(buffer));
            valread = read(sock, buffer, BUFSIZ);
            fprintf(stdout, "Username : ");
            fscanf(stdin, "%s", username);
            fprintf(stdout, "Password : ");
            fscanf(stdin, "%s", password);
            sprintf(authentication_data, "%s:%s", username, password);
            // memset(username, 0, sizeof(username));
            // memset(password,0,sizeof(password));
            send(sock, authentication_data, strlen(authentication_data), 0);
            memset(buffer,0,sizeof(buffer));
            valread = read(sock, buffer, BUFSIZ);
            ....
       else if (strcmp(temp, "register")==0){
            send(sock, REGISTER, strlen(REGISTER), 0);
            memset(buffer, 0, sizeof(buffer));
            valread = read(sock, buffer, BUFSIZ);
            
            fprintf(stdout, "Username : ");
            fscanf(stdin, "%s", username);
            fprintf(stdout, "Password : ");
            fscanf(stdin, "%s", password);
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
```

b. Sistem memiliki sebuah database yang bernama files.tsv. Isi dari files.tsv ini adalah path file saat berada di server, publisher, dan tahun publikasi. Setiap penambahan dan penghapusan file pada folder file yang bernama  FILES pada server akan memengaruhi isi dari files.tsv. Folder FILES otomatis dibuat saat server dijalankan. 
```
FILE *fp;
fp = fopen("file.tsv", "a+");

penulisan publisher, tahun publikasi, filepath ada di proses add
```

c. add
untuk command-command yang ada apa yang dilakukan client hampir sama yakni mengirim request dan mendapatkan apa yang direqueskan dari server.  
sehingga disini lebih fokus ke penanganan server
```
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
    }
    fclose(received_file);
}
```
d. download
```
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
```
e. delete
```
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
```
f. see
```
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
```
g. find
```
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
```
h. log  
setiap log terjadi proses yang sama tergantung dengan command yang diberikan
```
FILE *fp_log;
fp_log = fopen("running.log", "a+");
fprintf(fp_log, "Tambah: %s (%s)\n", filename, request);
fclose(fp_log);
```


### **Kendala**
1. Proses download masih belum bisa mendeteksi letak dari file

## SOAL 2  
### **Pembahasan**
### **Kendala**

## SOAL 3  
### **Pembahasan**
### **Kendala**
