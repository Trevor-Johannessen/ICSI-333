#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

void object(char[]); // creates o files
void lib(char[]); // creates a files
void remove_backslash(char*); // removes slashes from file paths
void compile(char[]); // creates executable
void cleanup(char[]); // cleans lingering files
char *get_ext(char*); // gets file extension


int main(int argc, char *argv[])
{
	char path[255];
	char safePath[255];

	printf("Please input directory to compile\n"); // get directory to compile
	scanf("%s", path);
	strcpy(safePath, path);
	
	object(path); // create o files
	object(path); // move o files
	lib(path); // create a libraries
	compile(safePath); // compile the o and a files into an executable
	cleanup(path); // remove files from top directory
	
	return 0;
}

// Recursivly decends through the folders creating .o files and moving them to the correct location
void object(char name[])
{
	int err = 0;

	char*gccArgs[5] = {"gcc", "SomeFileName", "-c", "somefilename", NULL}; // args to compile .o files
	char*mvArgs[4] = {"mv", "source", "new location", NULL}; // args to move files
	
	

	DIR *dir;
	struct dirent *sd;
	
	dir = opendir(name); // open directory
	
	
	
	char filePath[200];
	strcpy(filePath, name); // add the current directory to the file path
	strcat(filePath, "/"); // prep for next directory
	
	if(dir == NULL)
	{
		printf("Cant open dir\n");
	}
	else
	{
		while((sd=readdir(dir)) != NULL) // while there are still files
		{
			if((get_ext(sd -> d_name) == "") && (strcmp(sd -> d_name, "." )!= 0)) // if it is a directory
			{
				char nextFilePath[200]; // create new path to next directory
				strcpy(nextFilePath, filePath);
				strcat(nextFilePath, sd -> d_name);
				object(nextFilePath); // open next directory
			}
			else if(strcmp(get_ext(sd -> d_name), "c") == 0) // if its a .c file
			{
			
				char tempPath[255]; // create a path with the file
				strcpy(tempPath, filePath);
				strcat(tempPath,sd -> d_name);
				
				char tempStr[255]; // create a temp path which includes the file
				strcpy(tempStr, tempPath);
				tempStr[strlen(tempStr)-2] = 0; // remove the .c extension
				
				
				gccArgs[1] = tempPath;
			
				gccArgs[3] = NULL;
				
				// this was originally done with exec() and an array using fork()
				// When I made it use only one string at just cat on each part it errored
				// I ran out of time. 
				char strGccArgs[255];
				strcpy(strGccArgs, gccArgs[0]);
				strcat(strGccArgs, " ");
				strcat(strGccArgs, gccArgs[1]);
				strcat(strGccArgs, " ");
				strcat(strGccArgs, gccArgs[2]);
				
				
				err = system(strGccArgs); // compile .o file
				if(err != 0) // check if it compiled sucessfully
				{
					char str[511]; // standard output
					setbuf(stderr, str);
					exit(0); // stop program if file doesnt compile
				}
				
				
				// moving the .o file
				
				
				char fileName[255]; // new file path with the new .o file
				strcpy(fileName, sd -> d_name);
				fileName[strlen(fileName)-2] = 0;
				strcat(fileName, ".o");
				
				// mv fileName filePath
				mvArgs[1] = fileName;
				mvArgs[2] = filePath;
				
				// assemble the mv arguments
				// this was originally done with exec() and fork()
				char strMvArgs[255];
				strcpy(strMvArgs, mvArgs[0]);
				strcat(strMvArgs, " ");
				strcat(strMvArgs, mvArgs[1]);
				strcat(strMvArgs, " ");
				strcat(strMvArgs, mvArgs[2]);
				

				
				err = system(strMvArgs); // moves the file
				if(err != 0)
				{
					char str[511];
					setbuf(stderr, str); // standard output
				}
			}
		}
	
	
	}
	
	closedir(dir); // close directory
}

// creates static libraries from the .o files
void lib(char name[])
{
	int err = 0;
	char fileName[255]; 
	strcpy(fileName, name);
	remove_backslash(fileName); // remove slashes from the file path
	
	
	// Arguments for creating .a files
	char arArgs[255];
	strcpy(arArgs, "ar -rc "); // these args are always present
	strcat(arArgs,strcat(fileName, ".a ")); // adds "fileName.a";
	
	// this command can take in an unknown amount of parameters,
	// place determins where to place the terminating null
	int place = 3; 
	
	DIR *dir;
	struct dirent *sd;
	
	dir = opendir(name); // open directory
	
	
	
	char filePath[255]; // the file path to be amended
	strcpy(filePath, name);
	strcat(filePath, "/");
	
	if(dir == NULL)
	{
		printf("Cant open dir\n");
	}
	else
	{
		while((sd=readdir(dir)) != NULL) // go through all files
		{

			if((get_ext(sd -> d_name) == "") && (strcmp(sd -> d_name, "." )!= 0)) // if the file is a directory
			{
				char nextFilePath[255]; // the next file path
				strcpy(nextFilePath, filePath); // create path to next directory
				strcat(nextFilePath, sd -> d_name);
				lib(nextFilePath); // open the next directory
			}
			else if(strcmp(get_ext(sd -> d_name), "o") == 0) // if the file is a .o file
			{	
				strcat(arArgs, filePath);
				strcat(arArgs, sd -> d_name); // add it to the list of .o files in the directory
				strcat(arArgs, " ");
			}
		}
	
	}
		
	err = system(arArgs); // create the .a file
	if(err != 0) // if it fails to compile
	{
		char str[511];
		setbuf(stderr, str); // standard output
		exit(0);
	}
	
	char cpArgs[255]; // args to copy the new library to the correct folder
	strcpy(cpArgs, "cp "); // standard arguments for copying files
	strcat(cpArgs, fileName); // build the file path
	strcat(cpArgs, strcat(name, "/..")); // the folder to move to will always be the previous folder
	
	err = system(cpArgs);
	if(err != 0) // if it fails
	{
		char str[511];
		setbuf(stderr, str); // standard output
		printf("Error: %s\nerr = %d\n", str, err);
	}
	
	closedir(dir); // close directory
}

// compile the .a and .o files into an executable
void compile(char name[])
{
	int err = 0;
	// this command has unknown arguments so it uses the same place variable to place the NULL terminator. 
	char cmpArgs[255];
	strcpy(cmpArgs, "gcc "); // first argument
	
	DIR *dir;
	struct dirent *sd;
	
	
	
	
	dir = opendir(name);
	while((sd=readdir(dir)) != NULL) // while there are still files
	{
		if(strcmp(get_ext(sd -> d_name), "o") == 0)
		{
			char cpyArgs[255]; // copy args
			strcpy(cpyArgs, "cp "); // copy command
			strcat(cpyArgs, name); // name of path
			strcat(cpyArgs, "/"); // add slash to add file
			strcat(cpyArgs, sd -> d_name); // name of file
			strcat(cpyArgs, " .");// move to directory where the executable is
			err = system(cpyArgs); // execute copy
			if(err != 0)
			{
				char str[511]; // error stuff
				setbuf(stderr, str);
				printf("Error: %s\n", str);
			}
			
		}
	}
	
	
	
	closedir(dir); // close directory
	
	
	
	
	dir = opendir("."); // open directory that contains the current executable
	
	while((sd=readdir(dir)) != NULL) // goes through files finding all .a and .o files. 
		{
			if(strcmp(get_ext(sd -> d_name), "o") == 0) // if it finds a .o file
			{
					strcat(cmpArgs, sd -> d_name); // adds the .o file to the list of arguments
					strcat(cmpArgs, " ");
			}
			else if(strcmp(get_ext(sd -> d_name), "a") == 0) // if it finds a .a file
			{
					strcat(cmpArgs, sd -> d_name); // adds the .a file to the list of arguments
					strcat(cmpArgs, " ");
			}
		}
		

		// adds the rest of the arguments
		strcat(cmpArgs, "-o "); // compile into an executable

		strcat(cmpArgs, name); // name output as source directory
		strcat(cmpArgs, "Output"); // since both executables and directorys have no extension, this will cause a conflict. Add output to fix.


		
		
		err = system(cmpArgs); // compiles the file. 
		if(err != 0)
		{
			char str[511]; // more error stuff
			setbuf(stderr, str);
			printf("Error: %s\nerr = %d\n", str, err);
		}
		

	closedir(dir); // close directory
}

// cleans up and lingering .a or .o files.
void cleanup(char name[])
{
	int err = 0;

	char rmArgs[255] = "rm "; // remove args
	
	DIR *dir;
	struct dirent *sd;
	
	dir = opendir("."); // open directory where the file was executed
	
	while((sd=readdir(dir)) != NULL) // while there are files in the directory
		{
			if((strcmp(get_ext(sd -> d_name), "o") == 0) || (strcmp(get_ext(sd -> d_name), "a") == 0)) // finds .a and .o files and removes them
			{
				strcat(rmArgs, sd -> d_name); // add the file to remove to args
				
				
				err = system(rmArgs); // remove the file
				if(err != 0)
				{
					char str[511];
					setbuf(stderr, str);
					printf("Error: %s\n", str);
				}	
				strcpy(rmArgs, "rm "); // reset the args to be used again
			}
		}	
}

void remove_backslash(char* in) // I like passing in pointers now, it is very cool
{
	char *str = in;
	for(int i = 0; i < strlen(str); i++) // go through string and replace '/' and '\' with '-'
		if((str[i] == '/') || (str[i] == '\\'))
			str[i] = '-';
		
}

char *get_ext(char *in) {
	// strrchr returns a pointer to the last of a character in a string
	// returns NULL if there isn't any of that character
    char *str = strrchr(in, '.');
    if((str == NULL) || (str == in))
		return ""; // check if it has that character, if it does check if it in a valid position
    else
		return str + 1; // return the file extension (1 space past the dot)
}

