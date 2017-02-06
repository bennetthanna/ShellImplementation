#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>

//DID YOU FREE MEMORY AFTER ALLOCATING IT
//DID YOU CLOSE FILES AFTER OPENING

std::vector <char*> commandArguments;
std::vector <char*> filesInDirectory;
char input[1000];

int main(int argc, char *argv[]) {
    //if there is no command line argument use default ./my_shell prompt
    if (argc == 1) {
        fprintf(stdout, "my_shell> ");
    //if there is command line argument then use that argument as prompt
    } else if (argc == 2) {
        fprintf(stdout, "%s> ", argv[1]);
    //else print usage error
    } else {
        fprintf(stderr, "Usage: ./my_shell or ./my_shell PROMPT\n");
        exit(1);
    }
    
    //while user inputs commands
    while (fgets(input, sizeof(input), stdin) != NULL) {
        
        //if they enter exit then exit successfully
        if (strcmp(input, "exit\n") == 0) {
            exit(0);
        }
        
        //parse user input and push into commandArguments vector
        char *token;
        token = strtok(input, " \n");
        while (token != NULL) {
            commandArguments.push_back(token);
            token = strtok(NULL, " \n");
        }
        
        //if their first command is echo then echo out what comes after
        if (strcmp(commandArguments[0], "echo") == 0) {
            for (int i = 1; i < commandArguments.size(); ++i) {
                printf("%s ", commandArguments[i]);
            }
            printf("\n");
        }
        
        //if their first argument is cat
        if (strcmp(commandArguments[0], "cat") == 0) {
            //if the second command is file redirection
            if (strcmp(commandArguments[1], "<") == 0) {
                //  **TO DO**
                //DO FILE REDIRECTION SHIT?
            } else {
                //stat the file to get size of file
                struct stat statBuff;
                //error check stat
                if (lstat(commandArguments[1], &statBuff) < 0) {
                    printf("Error: %s\n", strerror(errno));
                //error check to make sure user has read access
                } else if (!S_IRUSR & statBuff.st_mode) {
                    printf("Cannot open file. Must have read access\n");
                } else {
                    //malloc fileContentsBuffer
                    char *fileContentsBuffer = (char*)malloc(sizeof(char) * statBuff.st_size);
                    if (fileContentsBuffer == NULL) {
                        fprintf(stderr, "Failed to allocate memory.\n");
                    } else {
                        //open file, get contents into buffer, put contents to stdout
                        FILE *file;
                        file = fopen(commandArguments[1], "r");
                        if (file == NULL) {
                            fprintf(stderr, "Error opening file\n");
                        }
                        
                        while (!feof(file))
                        {
                            // read in the line and make sure it was successful
                            if (fgets(fileContentsBuffer, statBuff.st_size, file) != NULL)
                            {
                                printf("%s", fileContentsBuffer);
                            }
                        }
                    }
                }
            }
        }
        
        if (commandArguments.size() >= 2) {
            //if the first command is ls AND the second command is -l
            if ((strcmp(commandArguments[0], "ls") == 0) && (strcmp(commandArguments[1], "-l") == 0)) {
                
                int total = 0;
                
                //References: http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
                //grab all files in directory and push back into vector
                DIR *d;
                struct dirent *dir;
                d = opendir(".");
                if (d) {
                    while ((dir = readdir(d)) != NULL) {
                        if (dir->d_type == DT_REG && dir->d_name[0] != '.') {
                            filesInDirectory.push_back(dir->d_name);
                        }
                    }
                    closedir(d);
                } else {
                    fprintf(stderr, "Could not open directory\n");
                }
                
                //iterate through vector and stat each file
                //add the size of each files block to total
                for (int i = 0; i < filesInDirectory.size(); ++i) {
                    struct stat sb;
                    if(lstat(filesInDirectory[i], &sb) < 0) {
                        printf("Error: %s\n", strerror(errno));
                    } else {
                        total += sb.st_blocks;
                    }
                }
                
                //print out total
                printf("total %i\n", total);
                
                //iterate through each file in vector and stat it
                for (int i = 0; i < filesInDirectory.size(); ++i) {
                    struct stat sb;
                    
                    if(lstat(filesInDirectory[i], &sb) < 0) {
                        printf("Error: %s\n", strerror(errno));
                    } else {
                        
                        //convert time to readable
                        char buffer[12];
                        struct tm* tm_info;
                        tm_info = localtime(&sb.st_mtime);
                        
                        strftime(buffer, 24, "%b %e %R", tm_info);
                        
                        //convert ID numbers to ID names
                        struct group *grp;
                        struct passwd *pwd;
                        grp = getgrgid(sb.st_gid);
                        pwd = getpwuid(sb.st_uid);
                        
                        //print out all information
                        printf("%-c%c%c%c%c%c%c%c%c%c  %i %-10s %-10s %6lli %-12s %-s\n",
                               S_ISREG(sb.st_mode) ? '-' :
                               S_ISDIR(sb.st_mode) ? 'd' :
                               S_ISCHR(sb.st_mode) ? 'c' :
                               S_ISBLK(sb.st_mode) ? 'b' :
                               S_ISFIFO(sb.st_mode) ? 'p' :
                               S_ISLNK(sb.st_mode) ? 'l' :
                               S_ISSOCK(sb.st_mode) ? 's' : '?',
                               (S_IRUSR & sb.st_mode) ? 'r' : '-',
                               (S_IWUSR & sb.st_mode) ? 'w' : '-',
                               (S_IXUSR & sb.st_mode) ? 'x' : '-',
                               (S_IRGRP & sb.st_mode) ? 'r' : '-',
                               (S_IWGRP & sb.st_mode) ? 'w' : '-',
                               (S_IXGRP & sb.st_mode) ? 'x' : '-',
                               (S_IROTH & sb.st_mode) ? 'r' : '-',
                               (S_IWOTH & sb.st_mode) ? 'w' : '-',
                               (S_IXOTH & sb.st_mode) ? 'x' : '-',
                               sb.st_nlink,
                               pwd->pw_name,
                               grp->gr_name,
                               sb.st_size,
                               buffer,
                               filesInDirectory[i]);
                    }
                }
            }
        
        //else if they just want to list not in long format
        } else if (strcmp(commandArguments[0], "ls") == 0) {
            //References: http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
            DIR *d;
            struct dirent *dir;
            d = opendir(".");
            if (d) {
                //grab each file in directory and print out file name
                while ((dir = readdir(d)) != NULL) {
                    if (dir->d_type == DT_REG && dir->d_name[0] != '.') {
                        printf("%s   ", dir->d_name);
                    }
                }
                closedir(d);
                printf("\n");
            } else {
                fprintf(stderr, "Could not open directory\n");
            }
        }
        
        //clear commandArguments vector
        commandArguments.clear();
        
        //return to command prompt
        if (argc == 1) {
            fprintf(stdout, "my_shell> ");
        } else if (argc ==2) {
            fprintf(stdout, "%s> ", argv[1]);
        }
    }
    
}
