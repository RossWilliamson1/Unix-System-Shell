/**************************************************************************
 * Assessment Title: Assessed Coursework Exercise #4
 *      (WITH STAGE 9)
 *
 *
 * Number of Submitted C Files: 2
 *
 * 
 * Date: 18/03/2019
 *
 * 
 * Authors: 
 *	1. Courtney Brown, Reg no: 201707475
*	2. Ross Williamson, Reg no: 201707864
 *	3. Connor Bell, Reg no: 201708244
 *	4. Brian Milloy, Reg no: 201707912
 *	5. Tereze Strazdina, Reg no: 201735788
 * 
 *
 *	Statement: We confirm that this submission is all our own work.
 *
 *      (Signed) COURTNEY BROWN	
 *	
 * 	(Signed) ROSS WILLIAMSON
 *	
 *	(Signed) CONNOR BELL
 *	
 *	(Signed) BRIAN MILLOY
 *
 *	(Signed) TEREZE STRAZDINA
 *
 **************************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void getInput();
void processInput();
void token();
void initialiseFile();
void changeDirectory();
void getPath();
void setPath();
void history();
void lastCommand();
void alias();
void commandNumber();
void saveHistory();
void saveAlias();
void checkAlias();

char userInput[512]; 				// Stores original user input
char * storeInput[512]; 			// Stores user input once it has been split into tokens
char * historyArray[20]; 			// Stores the history of commands. It is re-filled every time the shell is launched.
char * path;
int keepGoing = 1;

	             
// Alias struct
typedef struct Aliases aliases;
struct Aliases {
	char aliasName[512];
	char command[512];
};
aliases aliasArray[512];

/* Main function */
int main() {
	char cwd[256];
	// Saves the current path
	path = getenv("PATH");
	printf("Saving path: %s\n", path);
	
	// Prints the home environment
	chdir(getenv("HOME"));
	printf("Home: %s\n",getcwd(cwd, sizeof(cwd)));

	// Files are initialised
	initialiseFile();
	while(keepGoing) {
		getInput();
	}

	// Path is restored on exit
	printf("Program will terminate now.\n");
	setenv("PATH", path, 1);
	printf("Path restored to: %s\n", path);
	
	return 0;
}

/* Initialises the history and alias files */
void initialiseFile() {
	FILE * historyFile; 				// File that contains the user's history
	FILE * aliasFile;			       // File that contains the list of aliases
	char tempString[512];
	strcpy(tempString, getenv("HOME"));
	strcat(tempString, "/.hist_list");
	historyFile = fopen(tempString, "r+");

	// Opens the file containing history in the home directory
	if (historyFile == NULL) {
		printf("Creating file...\n");
		historyFile = fopen(tempString, "w+");
	}
	
	// Transfers all of the data from the history file into an array and closes the history file
	for (int count = 0; count < 20; count++) {
		char tempArray[512];
		if (fgets(tempArray, 512, historyFile) == NULL) {
			break;
		}
		historyArray[count] = strdup(strtok(tempArray,"\n"));

	}
	fclose(historyFile);

	// Opens the file containing aliases in the home directory
	int c = 0;
	char anotherTemp[512];
	strcpy(anotherTemp, getenv("HOME"));
	strcat(anotherTemp, "/.aliases");
	aliasFile = fopen(anotherTemp, "r+");
	if(aliasFile == NULL){
		printf("Creating alias file...\n");
		aliasFile = fopen(anotherTemp, "w+");
	}

	// Transfers all of the data from the alias file into an array and closes the alias file
	else {
		
		for (int e = 0; e < 10; e++) {
			char temparray[512];
			fscanf(aliasFile, "%s %[^\n]s \n", aliasArray[e].aliasName, aliasArray[e].command);
		}
	}
	fclose(aliasFile);
}

/* Reads in user input to the shell */
void getInput() {
	char temp[512]; // Needed in order to shift historyArray when it is full
	
	printf("$$$: "); // Prompt
	if (fgets(userInput, sizeof(userInput), stdin) != NULL) {
			if (strlen(userInput) > 1) {
				userInput[strlen(userInput) - 1] = '\0';
			}
			else getInput();
	}
	else {
		setenv("PATH", path, 1);
		printf("Path restored to: %s\n", path);
		printf("\n");
		saveHistory();
		saveAlias();
		exit(0);
	}
	
	// If the history array is full, everything in the array is moved down one place to make room for the new command
	// If the array is not full to begin with, a loop checks for the first available space and puts the latest command in there
	if (historyArray[19] == NULL) {
		for (int c=0; c<21; c++) {
			if (historyArray[c] == NULL) {
				if(strcmp(userInput, "exit") != 0 && strncmp(userInput, "!", 1) != 0) {
					historyArray[c] = strdup(userInput);
				}
				break;
			}
		}
	}
	else {

		if(strcmp(userInput, "exit") != 0 && strncmp(userInput, "!", 1) != 0){
			int counter = 20;
			strcpy(temp , historyArray[0]);
    			for(int i=0; i<counter-1; i++) {
				historyArray[i]=historyArray[i+1];
				
	    		}
			historyArray[counter-1] = strdup(userInput);								
		}
	}
	for(int a = 0; a < 10; a++){
		checkAlias();
	}
	token();
}

/* Tokenizes the user input */
void token() {
	char * split;
	char * pointer;
	char * ptr = userInput;
	int counter = 0;
	split = strtok(ptr, " ");
	storeInput[0] = split; // The first token (cd, getPath etc) is stored in the storeInput array
	
 	// Loop to store all of the tokens in the storeInput array. Only ends when there are no tokens left.
	// First token is stored in storeInput[0], then the second is stored in storeInput[1], then storeInput[2] and so on.
	while (split) {	
		counter++;
		split = strtok(NULL, " |><&;\t");
		storeInput[counter] = split;
	}
	counter++;
	storeInput[counter] = '\0';
	processInput();
}

/* Processes the user input */
void processInput() {
	char commandMinus[1];
	char commandMinusTwo[1];
	int find = 0;
	int found = 0;
	int tempInt = 0;
	int tempIntAgain = 0;
	
	// This block finds out which command the user entered and calls the corresponding function
	if (strcmp("cd", storeInput[0]) == 0) {
		//printf("Changing to something...\n");
		changeDirectory();
	}
	
	else if (strcmp("getPath", storeInput[0]) == 0 || strcmp("getpath", storeInput[0]) == 0) {
		getPath();
	}
	
	else if (strcmp("setPath", storeInput[0]) == 0 || strcmp("setpath", storeInput[0])== 0) {
		setPath();
	}
	
	else if (strcmp("history", storeInput[0]) == 0) {
		history();
	}
	
	else if (strcmp("!!", storeInput[0]) == 0) {
		lastCommand();
	}
	
	// If the first 2 characters of the command are "!-" then try running the appropriate function
	else if (strcspn(storeInput[0], "!") == 0 && strcspn(storeInput[0], "-") == 1) {
		int lenCheck = strlen(userInput); //Just checking whether the command has a number to use. If not, invalid.
		if (lenCheck != 3 && lenCheck != 4) {
			printf("Invalid Command. Please enter '!-<Number between 1 and 20>'.\n");
		}
		else {
			if (lenCheck == 3) {
				commandMinus[0] = userInput[2];
				find = userInput[2] - '0'; // Convert char to int
			}
			else if (lenCheck == 4) {
				printf("%c\n", userInput[3]);
				commandMinus[0] = userInput[2];
				tempInt = commandMinus[0] - '0'; // Converting char to int so that the number entered by the user can be checked
				tempIntAgain = userInput[3] - '0'; // Converting char to int so that the number entered by the user can be checked
				
				
				if (tempInt == 2 && tempIntAgain != 0) {
					// If the user has entered a number in the 20s that is not '20', find is set to a value
					// that will display an error in the next conditional check
					find = 50;
				}
				else if (tempInt == 1) {
					// If the number entered is between 10 and 20, 'find' becomes that number
					find = tempIntAgain + 10;
				}
				else if (tempInt == 2 && tempIntAgain == 0) {
					// If none of the conditions so far have been met, the user has entered '20'
					// or the user has not entered a valid command
					find = 20;
				}
					
			}
			if (find < 0 || find > 20) {
				perror("History only holds 20 commands. Please enter a number between 1 and 20.\n");
			}
			else {
			// Find the current command in history. The first spot that has not been filled in the array will be filled with this
				for (int i = 0; i < 22; i++) {
						if (historyArray[i] == NULL) {
							 found = i;
							 break;
						}
				}
				int minus = found - find;
				// If, after the subtraction, the user wants to run command -1 or something similar, an error is displayed
				if (minus < 0) {
					perror("Unable to run this command as the command number is invalid");
				}
				else {
					if (historyArray[minus] != NULL) {
						// Sets the command to the current command minus the number entered by the user
						strcpy(userInput,historyArray[minus]); 			
						for(int a = 0; a < 10; a++) {
						checkAlias();
						}
						token();
					}
					else {
						perror("There is no command corresponding to the number entered");
					}
				}
			}
		}
	}
	else if (strncmp(userInput, "!", 1) == 0 && strncmp(userInput, "-", 2) != 0) {
		commandNumber();
		
	}
	else if (strcmp("alias", storeInput[0]) == 0 || strcmp("unalias", storeInput[0]) == 0) {
		alias();
	}
	else if (strcmp("exit", storeInput[0]) == 0) {
		
		saveHistory();
		saveAlias();
		keepGoing = 0;
	}
	else if (storeInput[0] == NULL) {
		keepGoing = 0;
	}
	
	// Creates a variable and stores the int returned by the fork() function in it
	else {
		int processID;
		processID = fork();

		// If the fork() returns -1, then an error has occurred.
		if (processID == -1) {
			perror("The following error occured");
		}

		// If fork() returns 0, the child process is running and we can execute the system command (provided it is valid).
		else if (processID == 0) {
			if (execvp(storeInput[0], storeInput) == -1) {
				for (int c=0; c<512; c++) {
					if (storeInput[c] != NULL) {
						perror("The following error occurred");
						printf("Invalid command entered: %s\n", storeInput[c]);
					}
				}

			}
			exit(1); // Terminate current child process
		}
	
		// When the fork() is positive, the parent process is running.
		else {
			int waiting;
			(void)waitpid(processID, &waiting, 0);
		}
	} 
}


/* Displays shell prompt history */
void history() {
	if (storeInput[1] == NULL) {
		char current;

		// If the array is empty, there have been no previous commands
		if (historyArray[0] == NULL) { 
			printf("There is no history to display.\n"); 
		}
		else {
			for (int c = 0; c < 20; c++) {
				if (historyArray[c] == NULL) {
				}
				else {
					printf("%d: %s\n",c+1, historyArray[c]);	
				}
			}
		}
	}
	else {
		printf("Too many arguments entered.\n");
		printf("To show history simply type 'history'.\n");
	}
}

/* Gets and displays the current HOME and PATH environments to the user */
void getPath() {
	if(storeInput[1] == NULL){
		printf("The current HOME enviroment is: %s\n", getenv("HOME"));
		printf("The current PATH environment is: %s\n", getenv("PATH"));
	}
	else{
		printf("Too many arguments given. Please enter 'getpath' and try again.\n");
	}
}

/* Sets a new environment for the application to use */
void setPath() {
	if(storeInput[1] != NULL && storeInput[2] == NULL) {
		if(setenv("PATH", storeInput[1], 1) == 0) {
			printf("Path changed.\n");
		}
	}
	else if (storeInput[2] != NULL) {
		printf("Too many arguments entered. Please enter 'setpath <argument>' and try again.\n");
	}
	else if (storeInput[1] == NULL) {
		printf("Too few arguments entered. Please enter 'setpath <argument>' and try again.\n");
	}	
}

/* Changes the current directory */ 
void changeDirectory() {
	// Changes directory to the users Home directory
	if (storeInput[1] == NULL) {
		chdir(getenv("HOME"));
	}
	// If too many parameters are given by the user, a message is printed informing them of this 
	else if (storeInput[2] != NULL) {
		printf("Too many parameters given. Please enter 'cd <directory>' and try again.\n");
	}
	// Changes directory to user specified location 
	else {
		printf("Changing to: %s\n", storeInput[1]);
		if (chdir(storeInput[1]) == 0) {
		   printf("Directory changed.\n");
		}
		else {
			perror("The following error occured");
			printf("The file that caused this issue was: %s\n", storeInput[1]);
		}		
	}
}

/* Sets the last command entered by the user to the current command and calls the token() function to process it */
void lastCommand() {
	// Checks if the array is full. If it is, the last command will be the one in space 20
	if (historyArray[19] != NULL) { 
		strcpy(userInput,historyArray[19]);
		token();
	}
	else if (historyArray[0] == NULL) {
		printf("There is no history to execute.\n");
	}
	// If the array is not full, the loop finds the first available space then executes the last command
	else {
		for (int f = 0; f < 20; f++) {
			if (historyArray[f] == NULL) {
				strcpy(userInput,historyArray[f-1]);
				for(int a = 0; a < 10; a++){
		checkAlias();
	}
				break;
			}
		}
		token();	
	}	
}

/* Sets up aliases, removes aliases, invokes aliases and prints all aliases */
void alias() {
	int aliasNo = 0;
	int counter = 0;
	for (counter = 0; counter < 10; counter++) {
		if (aliasArray[counter].command[0] != '\0') {
			aliasNo++;
		}
	}
	// The user has entered "alias"
	if (strcmp("alias", storeInput[0]) == 0) {
		// If the array is empty, there have been no previous aliases
		if (storeInput[1] == NULL) {
			if (aliasNo == 0) {
				printf("There are no aliases to display.\n"); 
			}
			else {
				printf("There is/are %d alias(es)\n", aliasNo);
				for (int c = 0; c < 10; c++) {
					// Prints contents of alias array
					if (aliasArray[c].command[0] != '\0') {
						printf("%s - %s\n", aliasArray[c].aliasName, aliasArray[c].command);
					}
				}
			}
		}
		// If there are too many or too few arguments, instructions on how to create an alias correctly are displayed
		else if (storeInput[2] == NULL || storeInput[4] != NULL) {
			printf("Command is not recognised. To create an alias, please enter 'alias <name> <command>'.\n");
		}
		// Given that the entered command is valid, the program will create an alias
		else if (aliasNo != 10) {
			for (int l = 0; l < 512; l++) {
				// If the alias name entered already exists, it is overwritten
				if (strcmp(aliasArray[l].aliasName, storeInput[1]) == 0) {
					strcpy(aliasArray[l].aliasName, storeInput[1]);
					strcpy(aliasArray[l].command, storeInput[2]);
					if (storeInput[3] != NULL) {
						strcat(aliasArray[l].command, " ");
						strcat(aliasArray[l].command, storeInput[3]);
					}
					printf("Alias overwritten.\n");
					break;
				}
				// If the alias does not exist, it is created
				else if (aliasArray[l].command[0] == '\0') {
					strcpy(aliasArray[l].aliasName, storeInput[1]);
					strcpy(aliasArray[l].command, storeInput[2]);
					if (storeInput[3] != NULL) {
						strcat(aliasArray[l].command, " ");
						strcat(aliasArray[l].command, storeInput[3]);
					}
					printf("Alias created.\n");
					break;
				}
			}
		}
		else {
			// If the alias already exists, it is overwritten
			int done = 0;
			for(int l = 0; l< 512; l++){
				if (strcmp(aliasArray[l].aliasName,storeInput[1]) == 0) {
					strcpy(aliasArray[l].aliasName, storeInput[1]);
					strcpy(aliasArray[l].command, storeInput[2]);
					if (storeInput[3] != NULL) {
						strcat(aliasArray[l].command, " ");
						strcat(aliasArray[l].command, storeInput[3]);
					}
					printf("Alias overwritten.\n");
					done = 1;
					break;
				}
			}
			if(done == 0){

				printf("There are too many aliases, remove some to add more.\n");
			}
		}
	}
	else if (storeInput[1] == NULL || storeInput[2] != NULL) {
				printf("Command is not recognised. To create an alias, please enter 'alias <command>'.\n");
	}
	// The user has entered "unalias"
	else {
		if (storeInput[1] == NULL || storeInput[4] != NULL) {
			printf("Command is not recognised.\n");
		}
		else {
			// The user wants to unalias an entire command as opposed to removing a single alias
			if (storeInput[2] == NULL) {
				for (int c = 0; c < 512; c++) {
					// Given that the entered command is valid, the program will remove an alias
					// The program finds the command the user has entered in the aliasArray and NULLs it
					if (strncmp(aliasArray[c].command, storeInput[1], strlen(storeInput[1])) == 0) {
						aliasArray[c].command[0] = '\0';
						aliasArray[c].aliasName[0] = '\0';
						aliasNo--;
						printf("Alias successfully removed.\n");
						counter++;
					}
				}
				if(counter == 0) {
					if (aliasNo > 0) {
						printf("No such alias exists.\n");
					}
					else if (aliasNo == 0) {
						printf("There are no aliases to remove.\n");
					}
				}
			}
			// The user wants to unalias a single alias as opposed to an entire command
			else {  
				char * temp[512];
				strcat(temp, storeInput[1]);
				strcat(temp, " ");
				strcat(temp, storeInput[2]); // Temporary array used to store the second and third token entered by the user
				for (int b = 0; b < 512; b++) {
					if (strcmp(temp, aliasArray[b].command == 0)) {
						// NULLs the space in the array in the event that temp holds an entire command
						aliasArray[b].command[0] = '\0';
						aliasArray[b].aliasName[0] = '\0';
					}
					else if (strcmp(storeInput[1], aliasArray[b].aliasName == 0)) {
						// NULLs the space in the array in the event that temp holds a specific alias
						aliasArray[b].command[0] = '\0';
						aliasArray[b].aliasName[0] = '\0';
					}
				}	
			}
		}
	}	
}

/* Checks that the history command entered is valid */
void commandNumber() {
	int find = 0;
	char toIntAgain[1];
	char toInt[2];
	int lenCheck = strlen(userInput); // Checks that the command is the right length
		if (lenCheck != 2 && lenCheck != 3) {
			printf("History holds 20 commands. Please enter a number between 1 and 20.\n");	
		}
		else {
			if (lenCheck == 2) {
				toInt[0] = userInput[1];
				find = atoi(toInt); // Changes char to int
				
			}
			else if (lenCheck == 3) {
				toInt[0] = userInput[1];
				toIntAgain[0] = userInput[2];
				int tempOne = atoi(toInt); // Changes char to int
				int tempTwo = userInput[2] - '0';
				if (tempOne == 2 && tempTwo !=0) {
					// If the user has entered a number in the 20s that is not '20', find is set to a value
					// that will display an error in the next conditional check
					find = 50;
				}
				else if (tempOne == 1) {
					// If the number entered is between 10 and 20, 'find' becomes that number
					find = tempTwo + 10;
				}
				else if(tempOne == 2 && tempTwo == 0){
					find = 20;
				}
				else {
					printf("Invalid command, please enter a number between 1 and 20");
				}

			}
			if (find < 0 || find > 20) {
				// If none of the conditions so far have been met, the user has entered '20'
				printf("History holds 20 commands. Please enter a number between 1 and 20.\n");
			}
			else {
				// If the history command that the user wants to execute exists, then it is executed
				if (historyArray[find - 1] != NULL) {
					strcpy(userInput,historyArray[find - 1]);				
					for(int a = 0; a < 10; a++) {
						checkAlias();
					}
					token();
				}
				else {
					if (find == 0) {
						printf("This is not a valid history command, please invoke this command with a number between 1 and 20.\n");
					}
					else {
						printf("This number does not have a corresponding command.\n");
					}
				}
			}
		}	
}

/* Saves history to the .hist_list file */
void saveHistory() {
	FILE * historyFile; 				// File that contains the user's history
	int h = 0;
	char tempString[512];
	strcpy(tempString, getenv("HOME"));
	strcat(tempString, "/.hist_list");
	historyFile = fopen(tempString, "w+");
	if (historyFile == NULL) {
		printf("Could not open history file.");
		exit(5);
	}
	while (historyArray[h] != NULL) { 
		fprintf(historyFile, "%s\n", historyArray[h]);
		h++;
		if (h == 20) {
			break;
		}
	}
	fclose(historyFile);
} 

/* Saves aliases to the .aliases file */
void saveAlias() {
 	FILE * aliasFile;   // File that contains the list of aliases
	int a = 0;
	char anotherTemp[512];
	strcpy(anotherTemp, getenv("HOME"));
	strcat(anotherTemp, "/.aliases");
	aliasFile = fopen(anotherTemp, "w+");
	if (aliasFile == NULL) {
		printf("Could not open alias file.");
		exit(5);
	}
	while (aliasArray[a].command != NULL) { 
		fprintf(aliasFile, "%s %s \n",aliasArray[a].aliasName, aliasArray[a].command);
		a++;
		if (a == 10) {
			break;
		}
	}
	fclose(aliasFile);
}

/* For every command the user enters, this function checks if it is an alias  */
void checkAlias(){
	int lengthCount = 0; 
	int aliasNo = 0;
		for(int counter = 0; counter < 10; counter++){
			if(aliasArray[counter].command[0] != '\0'){
				aliasNo++;
			}
		}
	if (strncmp(userInput, "alias", 5) != 0 && strncmp(userInput, "unalias", 7) != 0) {
		for (int i=0; i<aliasNo; i++) {
		
				if (strncmp(userInput, aliasArray[i].aliasName, strlen(userInput)) == 0){ 
					strcpy(userInput, aliasArray[i].command);			
			}	
		
		}
	}	
	
}
