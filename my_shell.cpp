#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    char *line = NULL;
    char *token;
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
        
        //exit on success if user enters exit
        if (strcmp(line, "exit\n") == 0) {
            exit(0);
        }
        
        //parse the arguments and push into vector
        token = strtok(line, " \n");
        //error check strtok
        if (token == NULL) {
            perror("Strtok failure");
        }
        while (token != NULL) {
            commandArguments.push_back(token);
            token = strtok(NULL, " \n");
        }
        
        //count number of pipes
        numPipes = 0;
        for(int i = 0; i < commandArguments.size(); ++i) {
            if (strcmp(commandArguments[i], "|") == 0) {
                ++numPipes;
            }
        }
        
        //number of processes is pipes + 1
        int processCount = numPipes + 1;
        
        //create vector to hold file descriptors for the pipes
        std::vector<int *>FDs;
        
        //create new file descriptors for each pipe and push back into vector
        for (int i = 0; i < numPipes; ++i) {
            int *fileDescriptors = new int[2];
            fileDescriptors[0] = 0;
            fileDescriptors[1] = 1;
            FDs.push_back(fileDescriptors);
        }
        
        //call pipe on each pair of file descriptors
        for (int i = 0; i < FDs.size(); ++i) {
            pipe(FDs[i]);
        }
        
        //if there are no pipes
        if (numPipes == 0) {
            //fork
            switch(fork()) {
                //error check fork
                case -1:
                    perror("Fork failure");
                    exit(1);
                case 0:
                    //iterate through to see if there is file redirection
                    for (int i = 0; i < commandArguments.size(); ++i) {
                        if (strcmp(commandArguments[i], "<") == 0 || strcmp(commandArguments[i], ">") == 0 ||
                            strcmp(commandArguments[i], ">>") == 0 || strcmp(commandArguments[i], "2>") == 0 ) {
                            fileRedirection = true;
                        }
                    }
                    //grab the command before the file redirection symbol
                    for (int i = 0; i < commandArguments.size(); ++i) {
                        if (strcmp(commandArguments[i], "<") != 0 && strcmp(commandArguments[i], ">") != 0 &&
                            strcmp(commandArguments[i], ">>") != 0 && strcmp(commandArguments[i], "2>") != 0 ) {
                            command.push_back(commandArguments[i]);
                        } else {
                            break;
                        }
                    }
                    //convert vector of char* to char** to pass to exec
                    firstArg = (char**)malloc(sizeof(char*) * (command.size() + 1));
                    if (firstArg == NULL) {
                        perror("Malloc failure\n");
                    }
                    
                    for(int i = 0; i < command.size(); ++i) {
                        firstArg[i] = command[i];
                    }
                    firstArg[command.size()] = NULL;
                    
                    //if there is file redirection
                    //take the command after the file redirection symbol
                    //open the file
                    //dup it
                    //close it
                    if (fileRedirection == true) {
                        for (int i = 0; i < commandArguments.size(); ++i) {
                            if (strcmp(commandArguments[i], "<") == 0) {
                                fileToBeDuped1 = fopen(commandArguments[i + 1], "r");
                                if (fileToBeDuped1 == NULL) {
                                    perror("Fopen failure");
                                }
                                fileDescriptor1 = fileno(fileToBeDuped1);
                                if (fileDescriptor1 == -1) {
                                    perror("Fileno failure");
                                }
                                if (dup2(fileDescriptor1, 0) == -1) {
                                    perror("Dup failure");
                                }
                                fclose(fileToBeDuped1);
                            } else if (strcmp(commandArguments[i], ">") == 0) {
                                fileToBeDuped2 = fopen(commandArguments[i + 1], "w");
                                if (fileToBeDuped2 == NULL) {
                                    perror("Fopen failure");
                                }
                                fileDescriptor2 = fileno(fileToBeDuped2);
                                if (fileDescriptor2 == -1) {
                                    perror("Fileno failure");
                                }
                                if (dup2(fileDescriptor2, 1) == -1) {
                                    perror("Dup failure");
                                }
                                fclose(fileToBeDuped2);
                            } else if (strcmp(commandArguments[i], ">>") == 0) {
                                fileToBeDuped2 = fopen(commandArguments[i + 1], "a");
                                if (fileToBeDuped2 == NULL) {
                                    perror("Fopen failure");
                                }
                                fileDescriptor2 = fileno(fileToBeDuped2);
                                if (fileDescriptor2 == -1) {
                                    perror("Fileno failure");
                                }
                                if (dup2(fileDescriptor2, 1) == -1) {
                                    perror("Dup failure");
                                }
                                fclose(fileToBeDuped2);
                            } else if (strcmp(commandArguments[i], "2>") == 0) {
                                fileToBeDuped3 = fopen(commandArguments[i + 1], "w");
                                if (fileToBeDuped3 == NULL) {
                                    perror("Fopen failure");
                                }
                                fileDescriptor3 = fileno(fileToBeDuped3);
                                if (fileDescriptor3 == -1) {
                                    perror("Fileno failure");
                                }
                                if (dup2(fileDescriptor3, 2) == -1) {
                                    perror("Dup failure");
                                }
                                fclose(fileToBeDuped3);
                            }
                        }
                    }
                    //execvp the command
                    //even if there is not file redirection it will hit this
                    execvp(firstArg[0], firstArg);
                    //error check
                    perror("Execvp failure");
                default:
                    //parent should wait
                    //free memory
                    wait(&status);
                    free(firstArg);
            }
        
        //else if there are pipes
        } else {
            //for each process
            for (int i = 0; i < processCount; ++i) {
                
                std::vector<char *>vectorWhat;
                
                //push back all arguments before the pipe into vector
                //this will be our process
                for(int i = 0; i < commandArguments.size(); ++i) {
                    if (strcmp(commandArguments[i], "|") != 0) {
                        vectorWhat.push_back(commandArguments[i]);
                    } else {
                        break;
                    }
                }
                
                //if there is file redirection set fileRedirection to true
                for (int i = 0; i < commandArguments.size(); ++i) {
                    if (strcmp(commandArguments[i], "<") == 0 || strcmp(commandArguments[i], ">") == 0 ||
                        strcmp(commandArguments[i], ">>") == 0 || strcmp(commandArguments[i], "2>") == 0 ) {
                        fileRedirection = true;
                    }
                }
                
                std::vector<char *>noFileRedirection;
                
                //grab the commands before the file redirection symbol
                for (int i = 0; i < vectorWhat.size(); ++i) {
                    if (strcmp(commandArguments[i], "<") != 0 && strcmp(commandArguments[i], ">") != 0 &&
                        strcmp(commandArguments[i], ">>") != 0 && strcmp(commandArguments[i], "2>") != 0 ) {
                        noFileRedirection.push_back(commandArguments[i]);
                    } else {
                        break;
                    }
                }
                
                //convert to char** to be able to pass to exec
                char **whatArgs = (char**)malloc(sizeof(char*) * (noFileRedirection.size() + 1));
                for(int i = 0; i < noFileRedirection.size(); ++i) {
                    whatArgs[i] = noFileRedirection[i];
                }
                whatArgs[noFileRedirection.size()] = NULL;
                
                //fork
                switch(fork()) {
                    //error check fork
                    case -1:
                        perror("Bad Fork");
                        exit(1);
                    case 0:
                        //if there is file redirection do the same as you did before
                        if (fileRedirection == true) {
                            for (int i = 0; i < commandArguments.size(); ++i) {
                                if (strcmp(commandArguments[i], "<") == 0) {
                                    fileToBeDuped1 = fopen(commandArguments[i + 1], "r");
                                    if (fileToBeDuped1 == NULL) {
                                        perror("Fopen failure");
                                    }
                                    fileDescriptor1 = fileno(fileToBeDuped1);
                                    if (fileDescriptor1 == -1) {
                                        perror("Fileno failure");
                                    }
                                    if (dup2(fileDescriptor1, 0) == -1) {
                                        perror("Dup failure");
                                    }
                                    fclose(fileToBeDuped1);
                                } else if (strcmp(commandArguments[i], ">") == 0) {
                                    fileToBeDuped2 = fopen(commandArguments[i + 1], "w");
                                    if (fileToBeDuped2 == NULL) {
                                        perror("Fopen failure");
                                    }
                                    fileDescriptor2 = fileno(fileToBeDuped2);
                                    if (fileDescriptor2 == -1) {
                                        perror("Fileno failure");
                                    }
                                    if (dup2(fileDescriptor2, 1) == -1) {
                                        perror("Dup failure");
                                    }
                                    fclose(fileToBeDuped2);
                                } else if (strcmp(commandArguments[i], ">>") == 0) {
                                    fileToBeDuped2 = fopen(commandArguments[i + 1], "a");
                                    if (fileToBeDuped2 == NULL) {
                                        perror("Fopen failure");
                                    }
                                    fileDescriptor2 = fileno(fileToBeDuped2);
                                    if (fileDescriptor2 == -1) {
                                        perror("Fileno failure");
                                    }
                                    if (dup2(fileDescriptor2, 1) == -1) {
                                        perror("Dup failure");
                                    }
                                    fclose(fileToBeDuped2);
                                } else if (strcmp(commandArguments[i], "2>") == 0) {
                                    fileToBeDuped3 = fopen(commandArguments[i + 1], "w");
                                    if (fileToBeDuped3 == NULL) {
                                        perror("Fopen failure");
                                    }
                                    fileDescriptor3 = fileno(fileToBeDuped3);
                                    if (fileDescriptor3 == -1) {
                                        perror("Fileno failure");
                                    }
                                    if (dup2(fileDescriptor3, 2) == -1) {
                                        perror("Dup failure");
                                    }
                                    fclose(fileToBeDuped3);
                                }
                            }
                        }
                        
                        //if it is the first process
                        if (i == 0) {
                            for (int j = 0; j < FDs.size(); ++j) {
                                //close the read end
                                if (close(FDs[j][0]) == -1) {
                                    perror("Close read end failure");
                                }
                                //if it is not the first pipe
                                if (j != 0) {
                                    //close the write end
                                    if (close(FDs[j][1]) == -1) {
                                        perror("Close write end failure");
                                    }
                                }
                            }
                            //dup the write
                            if (dup2(FDs[i][1], 1) == -1) {
                                perror("Dup failure");
                            }
                            //exec the arguments
                            execvp(whatArgs[0], whatArgs);
                            perror("Execvp failure");
                        }
                        
                        //if it is one of the middle processes
                        if (i > 0 && i < numPipes) {
                            for (int j =0; j < FDs.size(); ++j) {
                                //close the write ends except for the pipe previous
                                if(j != (i - 1)) {
                                    if (close(FDs[j][0]) == -1) {
                                        perror("Close write end failure");
                                    }
                                }
                                //close read ends except for current pipe
                                if (j != i) {
                                    if (close(FDs[j][1]) == -1) {
                                        perror("Close read end failure");
                                    }
                                }
                            }
                            //dup write end
                            if (dup2(FDs[i][1], 1) == -1) {
                                perror("Dup failure");
                            }
                            //dup read end
                            if (dup2(FDs[i-1][0], 0) == -1) {
                                perror("Dup failure");
                            }
                            //exec the arguments
                            execvp(whatArgs[0], whatArgs);
                            perror("Execvp failure");
                        }
                        
                        //if it is the last process
                        if (i == processCount - 1) {
                            for (int j = 0; j < FDs.size(); ++j) {
                                //close all write ends except for current
                                if (j != (i - 1)) {
                                    if (close(FDs[j][0]) == -1) {
                                        perror("Close write end failure");
                                    }
                                }
                                //close all read ends
                                if (close(FDs[j][1]) == -1) {
                                    perror("Close read end failure");
                                }
                            }
                            //dup read end
                            if (dup2(FDs[i-1][0], 0) == -1) {
                                perror("Dup failure");
                            }
                            //exec arguments
                            execvp(whatArgs[0], whatArgs);
                            perror("Execvp failure");
                        }
                }
                
                //if it is not the last process erase the process + the pipe from the command arguments
                if (i != numPipes) {
                    commandArguments.erase(commandArguments.begin(), (commandArguments.begin() + vectorWhat.size() + 1));
                } else {
                    //else just erase the process because there is no pipe following
                    commandArguments.erase(commandArguments.begin(), (commandArguments.begin() + vectorWhat.size()));
                }
                //clear vector and free args
                vectorWhat.clear();
                free(whatArgs);
            }
            
            //parent close all read and write ends of all pipes
            for (int i = 0; i < numPipes; ++i) {
                if (close(FDs[i][0]) == -1) {
                    perror("Close read end failure");
                }
                if (close(FDs[i][1]) == -1) {
                    perror("Close write end failure");
                }
            }
            //wait for each process
            for (int i = 0; i < processCount; ++i) {
                wait(&status);
            }
        }
        //clear command arguments
        commandArguments.clear();
        //delete all FDs
        for (int i = 0; i < FDs.size(); ++i) {
            delete(FDs[i]);
        }
        
        //return to command prompt
        if (argc == 1) {
            fprintf(stdout, "my_shell> ");
        } else if (argc == 2) {
            fprintf(stdout, "%s> ", argv[1]);
        }
    }
}
