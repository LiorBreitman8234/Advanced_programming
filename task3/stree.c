#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

int file_number = 0;
int dir_number = 0;
// ANSI escape codes for colors
#define COLOR_RESET "\033[0m"
#define COLOR_FOLDER "\033[1;34m"    // Blue color for folders
#define COLOR_FILE "\033[0m"         // Default color for regular files
#define COLOR_EXECUTABLE "\033[1;32m" // Green color for executable files

void printFile(const char* path)
{
    struct stat fileStat;
    if (stat(path, &fileStat) == -1) {
        fprintf(stderr, "Error getting file status: %s\n", path);
        return;
    }

    // Print file permissions
    printf((S_ISDIR(fileStat.st_mode)) ? " d" : " -");
    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

    // Print user and group
    struct passwd* pw = getpwuid(fileStat.st_uid);
    if (pw) {
        printf(" %s", pw->pw_name);
    } else {
        printf(" Unknown");
    }

    struct group* grp = getgrgid(fileStat.st_gid);
    if (grp) {
        printf(" %s", grp->gr_name);
    } else {
        printf(" Unknown");
    }

    printf(" %ld",fileStat.st_size);
}
void printTree(const char* dirPath, int indent,int lastDir) {
    DIR* dir = opendir(dirPath);
    if (!dir) {
        fprintf(stderr, "Error opening directory: %s\n", dirPath);
        return;
    }

    struct dirent* entry;
    int counter = 0;
    while ((entry = readdir(dir)) != NULL) {
        counter++;
        if (strncmp(entry->d_name, ".",1) == 0) {
            continue;
        }
        

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirPath, entry->d_name);

        struct stat entryStat;
        if (stat(path, &entryStat) == -1) {
            fprintf(stderr, "Error getting file status: %s\n", path);
            continue;
        }
        struct dirent* next = readdir(dir);

        for (int i = 0; i < indent; i++) {
            // if(i%3 == 0)
            // {
            //     {
            //         printf("│");    
            //     }
            // }
            if(lastDir)
            {
                printf("   ");
            }
            else{
                printf("│   ");
            }
            
        }

        
        if (S_ISDIR(entryStat.st_mode)) {
            if(next == NULL)
            {
                printf("└──");
            }
            else
            {
                printf("├──");
            }
            printFile(path);
            printf(" %s%s%s\n",COLOR_FOLDER,entry->d_name,COLOR_RESET);
            printTree(path, indent + 1,next==NULL);
        } else if (entryStat.st_mode & S_IXUSR) {
             if(next == NULL)
            {
                printf("└──");
            }
            else
            {
                printf("├──");
            }
            printFile(path);
            printf(" %s%s%s\n",COLOR_EXECUTABLE,entry->d_name,COLOR_RESET);
        } else {
            if(next == NULL)
            {
                printf("└──");
            }
            else
            {
                printf("├──");
            }
            printFile(path);
            printf(" %s%s%s\n",COLOR_FILE,entry->d_name,COLOR_RESET);
        }
        if(next == NULL)
        {
            break;
        }
        rewinddir(dir);
        for(int i =0; i < counter;i++)
        {
            readdir(dir);
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        char pwd[1024];
        if (getcwd(pwd, sizeof(pwd)) != NULL) {
            printTree(pwd,0,0);
        } 
        else {
        perror("getcwd() error");
        return 1;
        }
    }
    else{
        const char* dirPath = argv[1];
        printTree(dirPath, 0,0);
    }


    return 0;
}
