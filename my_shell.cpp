#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    char *line = NULL;
    char *token;
    char **args;
    size_t n = 0;
    int status;
    std::vector <char *> commandArguments;
    FILE *fileToBeDuped1;
    int fileDescriptor1;
    FILE *fileToBeDuped2;
    int fileDescriptor2;
    FILE *fileToBeDuped3;
    int fileDescriptor3;
    bool fileRedirection = false;
    
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
    
    while(getline(&line, &n, stdin)) {
        
        if (strcmp(line, "exit\n") == 0) {
            exit(0);
        }
        
        token = strtok(line, " \n");
        while (token != NULL) {
            commandArguments.push_back(token);
            token = strtok(NULL, " \n");
        }
        
        args = (char**)malloc(sizeof(char*) * (commandArguments.size() + 1));
        
        for(int i = 0; i < commandArguments.size(); ++i) {
            args[i] = commandArguments[i];
        }
        args[commandArguments.size()] = NULL;
        
        for (int i = 0; i < commandArguments.size(); ++i) {
            if (strcmp(commandArguments[i], "<") == 0 || strcmp(commandArguments[i], ">") == 0 ||
                strcmp(commandArguments[i], ">>") == 0 || strcmp(commandArguments[i], "2>") == 0 ) {
                    fileRedirection = true;
                }
        }
        
        switch(fork()) {
            case -1:
                perror("Bad Fork");
                exit(1);
            case 0:
                
                if (fileRedirection == true) {
                    for (int i = 0; i < commandArguments.size(); ++i) {
                        if (strcmp(commandArguments[i], "<") == 0) {
                            fileToBeDuped1 = fopen(commandArguments[i + 1], "r");
                            fileDescriptor1 = fileno(fileToBeDuped1);
                            dup2(fileDescriptor1, 0);
                            fclose(fileToBeDuped1);
                        } else if (strcmp(commandArguments[i], ">") == 0) {
                            fileToBeDuped2 = fopen(commandArguments[i + 1], "w");
                            fileDescriptor2 = fileno(fileToBeDuped2);
                            dup2(fileDescriptor2, 1);
                            fclose(fileToBeDuped2);
                        } else if (strcmp(commandArguments[i], ">>") == 0) {
                            fileToBeDuped2 = fopen(commandArguments[i + 1], "w");
                            fileDescriptor2 = fileno(fileToBeDuped2);
                            dup2(fileDescriptor2, 1);
                            fclose(fileToBeDuped2);
                        } else if (strcmp(commandArguments[i], "2>") == 0) {
                            fileToBeDuped3 = fopen(commandArguments[i + 1], "w");
                            fileDescriptor3 = fileno(fileToBeDuped3);
                            dup2(fileDescriptor3, 2);
                            fclose(fileToBeDuped3);
                        }
                    }
                }
                
                execlp(args[0], args[0], NULL);
                perror("execlp failure");
            default:
                wait(&status);
                free(args);
        }
        
//        } else {
//            switch(fork()) {
//                case -1:
//                    perror("Bad Fork");
//                    exit(1);
//                case 0:
//                    execvp(args[0], args);
//                    perror("execvp failure");
//                default:
//                    wait(&status);
//                    free(args);
//            }
//        }

        //clear commandArguments vector
        commandArguments.clear();
        
        //return to command prompt
        if (argc == 1) {
            fprintf(stdout, "my_shell> ");
        } else if (argc == 2) {
            fprintf(stdout, "%s> ", argv[1]);
        }
    }
}
