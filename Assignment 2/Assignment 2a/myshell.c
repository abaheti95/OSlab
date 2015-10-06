#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <error.h>
#include <errno.h>
#include <time.h>

#define BUFFER 200


void prompt(){
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s -> ", cwd);
    else
        perror("getcwd() error");
    return;
}



int compare(char const *s1, char const *s2){
    int i = 0;
    // printf("%s,%s",s1, s2);
    for(i = 0; s2[i]; i++){
        if(s1[i] != s2[i])
            return -1;
    }
    return 0;
}


int changeDir(char *dst){
    char dest[BUFFER];
    strcpy(dest,dst);
    

    char currWDir[BUFFER];



    if(dst[0] == '\0'){
        chdir("/home");
    }
    if(dst[0] != '/'){
        getcwd(currWDir,sizeof(currWDir));
        strcat(currWDir,"/");
        strcat(currWDir, dest);
        int err = chdir(currWDir);
        if(err == -1)
            perror("");
    }else if(dst[0] == '\0'){
        
    }else{
        int err = chdir(dst);
        if(err == -1)
            perror("");
    }

    return 0;
}

void makeDir(char * file){
    int err;
    char *token;
    token = strchr(file,' ');
    *strchr(token,'\n')='\0';
    err = mkdir(token, S_IRWXU);
    if(err == -1){
        perror("");
    }
}

void remDir(char *file){
    int err;
    char *token;
    token = strchr(file,' ');
    *strchr(token,'\n')='\0';
    err = rmdir(token);
    if(err == -1){
        perror("");
    }
}

void listDir(){
    DIR *dir;
    struct dirent *st_dir;

    char currWDir[BUFFER];
    getcwd(currWDir,BUFFER);

    dir = opendir(currWDir);

    if(dir == NULL){
        perror("");
    }else{
        while((st_dir = readdir(dir)) != NULL){
            if(strcmp(st_dir->d_name,".") != 0 && strcmp(st_dir->d_name,"..") != 0)
                printf("%s\t",st_dir->d_name);
        }
        printf("\n");
    }
    closedir(dir);
}


void listDirL(){
    DIR *dir;
    struct dirent *st_dir;
    struct stat sb;

    char currWDir[BUFFER];
    getcwd(currWDir,BUFFER);

    dir = opendir(currWDir);


    if(dir != NULL){

        while((st_dir = readdir(dir)) != NULL){

            if(0 == stat(st_dir->d_name, &sb)){

                printf( (S_ISDIR(sb.st_mode)) ? "d" : "-");
                printf( (sb.st_mode & S_IRUSR) ? "r" : "-");
                printf( (sb.st_mode & S_IWUSR) ? "w" : "-");
                printf( (sb.st_mode & S_IXUSR) ? "x" : "-");
                printf( (sb.st_mode & S_IRGRP) ? "r" : "-");
                printf( (sb.st_mode & S_IWGRP) ? "w" : "-");
                printf( (sb.st_mode & S_IXGRP) ? "x" : "-");
                printf( (sb.st_mode & S_IROTH) ? "r" : "-");
                printf( (sb.st_mode & S_IWOTH) ? "w" : "-");
                printf( (sb.st_mode & S_IXOTH) ? "x" : "-");

                printf(" %d\t%d\t%s\t%s\n",(int)sb.st_nlink,(int)sb.st_size,ctime(&sb.st_atime),(char *)st_dir->d_name);

            }
        }
        closedir(dir);
    }else{
        perror("");
    }
}

void copy(char *src, char *dst){

    printf("src = %s\n dst = %s\n", src,dst);
    if(access(src, F_OK) == -1){
        perror("");
    }else if(access(src, R_OK) == -1){
        perror("");
    }else if(access(dst, F_OK) == -1){
        char c;

        FILE *f_src = fopen(src, "r");
        FILE *f_dst = fopen(dst, "w");

        while((c = getc(f_src)) != EOF){
            putc(c, f_dst);
        }
        fclose(f_src);
        fclose(f_dst);
    }else {
        if(access(dst, W_OK) == -1){
            perror("");
            return;
        }

        struct stat st;
        stat(src, &st);
        stat(dst, &st);
        time_t r = st.st_mtime;
        time_t w = st.st_mtime;
        double d = difftime(r, w);
        if(d >= 0.0){
            char c;
            FILE *f_src = fopen(src, "r");
            FILE *f_dst = fopen(dst, "w");

            while((c = getc(f_src)) != EOF){
                putc(c, f_dst);
            }

            fclose(f_src);
            fclose(f_dst);

        }else{
            printf("Modification time error");
        }
    }
}


void exec(char * buffer){
    char *args[15];
    int i = 0;
    for(i = 0; i < 15; i++){
        args[i] =(char *) malloc(sizeof(char) * 15);
    }
    
    *strchr(buffer,'\n')='\0';
    char * token = strtok(buffer, " \t");
    args[0] = token;
    int len = 1;
    while(token != NULL){
        token = strtok(NULL, " \t");
        args[len] = token;
        len++;
    }


    args[len] = NULL;

    if(strcmp(args[len - 2], "&") == 0){
        args[len - 2] = NULL;

        pid_t pid;
        int status;

        pid = fork();
        if(pid < 0){

        }else if(pid == 0){
            execv(args[0],args);
            printf("errror in execv\n");
            execvp(args[0], args);
            printf("error in execvp\n");
            exit(1);
        }
    }else{
        pid_t pid;
        int status;

        pid = fork();

        if(pid < 0){

        }else if(pid == 0){
            execv(args[0],args);
            printf("errror in execv\n");
            printf("here");
            execvp(args[0], args);
            printf("error in execvp\n");
            exit(1);
        }else{
            wait();
        }
    }

}



void caller(){
    char buffer[BUFFER];
    char currWDir[BUFFER];
    int err;
    char *token;
    token = strtok(buffer," \t");


    while(1){
        bzero(buffer, BUFFER);
        prompt();
        fgets(buffer, BUFFER, stdin);

        if(compare(buffer, "cd") == 0){
            token = strchr(buffer,' ');
            
            if(token != NULL){
                token += 1;
                *strchr(token,'\n')='\0';
                 // printf("here\n");
                changeDir(token);
            }else{
                chdir("/home");
            }
        }else if(compare(buffer,"mkdir") == 0){
            makeDir(buffer);
        }else if(compare(buffer,"pwd") == 0){
            getcwd(currWDir, BUFFER);
            printf("%s\n", currWDir);
        }else if(compare(buffer,"rmdir") == 0){
            remDir(buffer);
        }else if(compare(buffer,"ls") == 0){
            token = strtok(buffer," \t");
            char *token2 = strtok(NULL," \t");
            if(token2 == NULL){
                listDir();
            }else if(compare(token2, "-l") == 0){
                listDirL();
            }else{
                printf("Command not recognized!\n");
            }
        }else if(compare(buffer, "exit") == 0){
            exit(0);
        }else if(compare(buffer, "cp") == 0){
            token = strtok(buffer," \t");
            char *src = strtok(NULL," \t");
            char *dest = strtok(NULL," \t");
            copy(src, dest);
        }else{
            exec(buffer);
            exit(1);
        }
    }
}


int main() {
    // char com[BUFFER];
    caller();
    return 0;
}