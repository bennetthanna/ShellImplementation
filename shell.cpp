#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

std::vector <char*> commandArguments;
char input[1000];
std::vector <char*> leftOfFileRedirection;
std::vector <char*> rightOfFileRedirection;

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
        
        for (int i = 0; i < commandArguments.size(); ++i) {
            if (strcmp(commandArguments[i], "<") == 0) {
                printf("Redirecting a file to processes stdin\n");
                
                for (int j = 0; j < i; j++) {
                    printf("commandArguments[%i] = %s\n", j, commandArguments[j]);
                    leftOfFileRedirection.push_back(commandArguments[j]);
                }
                
                for (int k = i+1; k < commandArguments.size(); ++k) {
                    rightOfFileRedirection.push_back(commandArguments[k]);
                }
            }
            if (strcmp(commandArguments[i], ">") == 0) {
                printf("Redirecting process stdout to file\n");
                
                for (int j = 0; j < i; j++) {
                    printf("commandArguments[%i] = %s\n", j, commandArguments[j]);
                    leftOfFileRedirection.push_back(commandArguments[j]);
                }
                
                for (int k = i+1; k < commandArguments.size(); ++k) {
                    rightOfFileRedirection.push_back(commandArguments[k]);
                }
                
            }
            if (strcmp(commandArguments[i], ">>") == 0) {
                printf("Appending process stdout to file\n");
                
                for (int j = 0; j < i; j++) {
                    printf("commandArguments[%i] = %s\n", j, commandArguments[j]);
                    leftOfFileRedirection.push_back(commandArguments[j]);
                }
                
                for (int k = i+1; k < commandArguments.size(); ++k) {
                    rightOfFileRedirection.push_back(commandArguments[k]);
                }
                
            }
            if (strcmp(commandArguments[i], "2>") == 0) {
                printf("Redirecting process stderr to file\n");
                
                for (int j = 0; j < i; j++) {
                    printf("commandArguments[%i] = %s\n", j, commandArguments[j]);
                    leftOfFileRedirection.push_back(commandArguments[j]);
                }
                
                for (int k = i+1; k < commandArguments.size(); ++k) {
                    rightOfFileRedirection.push_back(commandArguments[k]);
                }
                
            }
            if (strcmp(commandArguments[i], "|") == 0) {
                printf("PIPE THAT SHIT\n");
            }
        }
        
        for (int m = 0; m < leftOfFileRedirection.size(); m++) {
            printf("leftOfFileRedirection[%i] = %s\n", m, leftOfFileRedirection[m]);
        }
        
        for (int n = 0; n < rightOfFileRedirection.size(); n++) {
            printf("rightOfFileRedirection[%i] = %s\n", n, rightOfFileRedirection[n]);
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
