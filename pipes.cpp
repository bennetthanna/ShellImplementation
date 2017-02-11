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
    char **firstArg;
    size_t n = 0;
    int status;
    std::vector <char *> commandArguments;
    std::vector <char *> command;
    int numPipes;
    
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
    
    while(getline(&line, &n, stdin) != -1) {
        
        if (strcmp(line, "exit\n") == 0) {
            exit(0);
        }
        
        token = strtok(line, " \n");
        while (token != NULL) {
            commandArguments.push_back(token);
            token = strtok(NULL, " \n");
        }
        
        numPipes = 0;
        for(int i = 0; i < commandArguments.size(); ++i) {
            if (strcmp(commandArguments[i], "|") == 0) {
                ++numPipes;
            }
        }

        int processCount = numPipes+1;
        
        std::vector<int *>FDs;
        
        for (int i = 0; i < numPipes; ++i) {
            int *fileDescriptors = new int[2];
            fileDescriptors[0] = 0;
            fileDescriptors[1] = 1;
            FDs.push_back(fileDescriptors);
        }
        
        for (int i = 0; i < FDs.size(); ++i) {
            pipe(FDs[i]);
        }
        
        if (numPipes == 0) {
            switch(fork()) {
                case -1:
                    perror("Bad Fork");
                    exit(1);
                case 0:

                    for (int i = 0; i < commandArguments.size(); ++i) {
                        if (strcmp(commandArguments[i], "<") == 0 || strcmp(commandArguments[i], ">") == 0 ||
                            strcmp(commandArguments[i], ">>") == 0 || strcmp(commandArguments[i], "2>") == 0 ) {
                                fileRedirection = true;
                            }
                    }

                    for (int i = 0; i < commandArguments.size(); ++i) {
                        if (strcmp(commandArguments[i], "<") != 0 && strcmp(commandArguments[i], ">") != 0 &&
                            strcmp(commandArguments[i], ">>") != 0 && strcmp(commandArguments[i], "2>") != 0 ) {
                                command.push_back(commandArguments[i]);
                        } else {
                            break;
                        }
                    }
                    
                    firstArg = (char**)malloc(sizeof(char*) * (command.size() + 1));
                    
                    for(int i = 0; i < command.size(); ++i) {
                        firstArg[i] = command[i];
                    }
                    firstArg[command.size()] = NULL;

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
                                fileToBeDuped2 = fopen(commandArguments[i + 1], "r+");
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
                    
                    execvp(firstArg[0], firstArg);
                    perror("execlp failure");
                default:
                    wait(&status);
                    free(args);
                    free(firstArg);
            }
        } else {


            for (int i = 0; i < processCount; ++i) {
                
                std::vector<char *>vectorWhat;
                
                for(int i = 0; i < commandArguments.size(); ++i) {
                    if (strcmp(commandArguments[i], "|") != 0) {
                        vectorWhat.push_back(commandArguments[i]);
                    } else {
                        break;
                    }
                }
                
                for (int i = 0; i < commandArguments.size(); ++i) {
                    if (strcmp(commandArguments[i], "<") == 0 || strcmp(commandArguments[i], ">") == 0 ||
                        strcmp(commandArguments[i], ">>") == 0 || strcmp(commandArguments[i], "2>") == 0 ) {
                        fileRedirection = true;
                    }
                }
                
                std::vector<char *>noFileRedirection;
                
                for (int i = 0; i < vectorWhat.size(); ++i) {
                    if (strcmp(commandArguments[i], "<") != 0 && strcmp(commandArguments[i], ">") != 0 &&
                        strcmp(commandArguments[i], ">>") != 0 && strcmp(commandArguments[i], "2>") != 0 ) {
                        noFileRedirection.push_back(commandArguments[i]);
                    } else {
                        break;
                    }
                }
                
                char **whatArgs = (char**)malloc(sizeof(char*) * (noFileRedirection.size() + 1));
                for(int i = 0; i < noFileRedirection.size(); ++i) {
                    whatArgs[i] = noFileRedirection[i];
                }
                whatArgs[noFileRedirection.size()] = NULL;
                
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
                                    fileToBeDuped2 = fopen(commandArguments[i + 1], "a");
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
                        
                        if (i == 0) {
                            for (int j = 0; j < FDs.size(); ++j) {
                                if (close(FDs[j][0]) == -1) {
                                    perror("Could not close read");
                                }
                                
                                if (j != 0) {
                                    if (close(FDs[j][1]) == -1) {
                                        perror("Could not close write");
                                    }
                                }
                            }
                            
                            if (dup2(FDs[i][1], 1) == -1) {
                                perror("Could not dup2");
                            }
                            
                            execvp(whatArgs[0], whatArgs);
                            perror("Could not execvp");
                        }
                        
                        if (i > 0 && i < numPipes) {
                            for (int j =0; j < FDs.size(); ++j) {
                                
                                if(j != (i - 1)) {
                                    if (close(FDs[j][0]) == -1) {
                                        perror("Could not close write");
                                    }
                                }
                                
                                if (j != i) {
                                    if (close(FDs[j][1]) == -1) {
                                        perror("Could not close read");
                                    }
                                }
                            }
                            
                            if (dup2(FDs[i][1], 1) == -1) {
                                perror("Could not dup2");
                            }
         
                            if (dup2(FDs[i-1][0], 0) == -1) {
                                perror("Could not dup2");
                            }
                            
                            execvp(whatArgs[0], whatArgs);
                            perror("Could not execvp");
                        }
                        
                        if (i == processCount - 1) {
                            for (int j = 0; j < FDs.size(); ++j) {
                                if (j != (i - 1)) {
                                    if (close(FDs[j][0]) == -1) {
                                        perror("Could not close read");
                                    }
                                }

                                if (close(FDs[j][1]) == -1) {
                                    perror("Could not close read");
                                }
                            }

                            if (dup2(FDs[i-1][0], 0) == -1) {
                                perror("Could not dup2");
                            }
                            
                            execvp(whatArgs[0], whatArgs);
                            perror("Could not execvp");
                        }
                }
                
                if (i != numPipes) {
                    commandArguments.erase(commandArguments.begin(), (commandArguments.begin() + vectorWhat.size() + 1));
                } else {
                    commandArguments.erase(commandArguments.begin(), (commandArguments.begin() + vectorWhat.size()));
                }
                
                vectorWhat.clear();
                free(whatArgs);
            }
            
            
            for (int i = 0; i < numPipes; ++i) {
                if (close(FDs[i][0]) == -1) {
                    perror("Could not close read");
                }
                if (close(FDs[i][1]) == -1) {
                    perror("Could not close read");
                }
            }
            
            for (int i = 0; i < processCount; ++i) {
                wait(&status);
            }
        }
        
        commandArguments.clear();
        
        //return to command prompt
        if (argc == 1) {
            fprintf(stdout, "my_shell> ");
        } else if (argc == 2) {
            fprintf(stdout, "%s> ", argv[1]);
        }
    }
}