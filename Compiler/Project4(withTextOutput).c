#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>


int loop = 0; 

char originalPath[255];


void object(char[]); // creates o files
void open(char[]); // creates a files
void remove_backslash(char*);
void compile(char[]); // creates executable
void cleanup(char[]);
char *get_ext(char*);


char *get_ext(char *in) {
	// strrchr returns a pointer to the last of a character in a string
	// returns NULL if there isn't any of that character
    char *str = strrchr(in, '.');
    if((str == NULL) || (str == in)) return ""; // check if it has that character, if it does check if it in a valid position
    return str + 1; // return the file extension
}

int main(int argc, char *argv[])
{
	char path[255];
	char safePath[255];

	printf("Please input directory to compile\n");
	scanf("%s", path);
	strcpy(safePath, path);
	
	printf("Path = %s\n", path);
	strcpy(originalPath, path);
	
	object(path); // create o files
	object(path); // move o files
	open(path); // create a libraries
	usleep(100); 
	compile(safePath); // compile the o and a files into an executable
	//sleep(1); // sleep because waitpid doesn't want to work or the system is slow
	cleanup(path); // remove files from top directory
	
	return 0; // ends my suffering
}
/*
	Recursivly decends through the folders creating .o files and moving them to the correct location
*/
void object(char name[])
{
	int err = 0;
	printf("\n\n\nloop %d\n\n\n\n", loop);
	loop++;
	
	char*gccArgs[5] = {"gcc", "SomeFileName", "-c", "somefilename", NULL};
	char*mvArgs[4] = {"mv", "source", "new location", NULL};
	
	
	printf("name = %s\n", name);
	DIR *dir;
	struct dirent *sd;
	
	dir = opendir(name);
	
	
	
	char filePath[200];
	strcpy(filePath, name);
	strcat(filePath, "/");
	
	if(dir == NULL)
	{
		printf("Cant open dir\n");
	}
	else
	{
		while((sd=readdir(dir)) != NULL)
		{
			printf("name : %s\n", sd -> d_name);
			printf("Extension is %s\n", get_ext(sd -> d_name));
			if((get_ext(sd -> d_name) == "") && (strcmp(sd -> d_name, "." )!= 0))
			{
				printf("Opening %s\n", sd -> d_name);
				char nextFilePath[200];
				strcpy(nextFilePath, filePath);
				strcat(nextFilePath, sd -> d_name);
				object(nextFilePath);
			}
			else if(strcmp(get_ext(sd -> d_name), "c") == 0)
			{
			
				printf("File %s is a .c file\n", sd -> d_name);
				char tempPath[200];
				strcpy(tempPath, filePath);
				strcat(tempPath,sd -> d_name);
				printf("File Location is %s\n", tempPath);
				
				char tempStr[100];
				strcpy(tempStr, tempPath);
				tempStr[strlen(tempStr)-2] = 0;
				printf("tempStr = %s\n", tempStr);
				
				
				gccArgs[1] = tempPath;
				
			
				//gccArgs[3] = tempPath;
				gccArgs[3] = NULL;
				
				char strGccArgs[255];
				strcpy(strGccArgs, gccArgs[0]);
				strcat(strGccArgs, " ");
				strcat(strGccArgs, gccArgs[1]);
				strcat(strGccArgs, " ");
				strcat(strGccArgs, gccArgs[2]);
				
				printf("strGccArgs = %s\n", strGccArgs);
				
				err = system(strGccArgs);
				if(err != 0)
				{
					char str[511];
					setbuf(stderr, str);
					printf("Error: %s\nerr = %d\n", str, err);
					exit(0);
				}
				
				char fileName[100];
				strcpy(fileName, sd -> d_name);
				fileName[strlen(fileName)-2] = 0;
				strcat(fileName, ".o");


				
				
				mvArgs[1] = fileName;
				mvArgs[2] = filePath;
				
				printf("mvArgs[1] = %s\n", mvArgs[1]);
				printf("mvArgs[2] = %s\n", mvArgs[2]);
				
				char strMvArgs[255];
				strcpy(strMvArgs, mvArgs[0]);
				strcat(strMvArgs, " ");
				strcat(strMvArgs, mvArgs[1]);
				strcat(strMvArgs, " ");
				strcat(strMvArgs, mvArgs[2]);
				
				printf("strMvArgs = %s\n", strMvArgs);
				
				err = system(strMvArgs);
				if(err != 0)
				{
					char str[511];
					setbuf(stderr, str);
					printf("Error: %s\nerr = %d\n", str, err);
				}
			}
		}
	
	
	}
	
	closedir(dir);
}

// creates static libraries from the .o files
void open(char name[])
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
	
	// print next directory
	printf("\n\n\nname = %s\n", name);
	DIR *dir;
	struct dirent *sd;
	
	dir = opendir(name); // open directory
	
	
	
	char filePath[255];
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
			printf("name : %s\n", sd -> d_name); // display name of current file
			printf("Extension is %s\n", get_ext(sd -> d_name)); // display file extension of current file
			if((get_ext(sd -> d_name) == "") && (strcmp(sd -> d_name, "." )!= 0)) // if the file is a directory
			{
				printf("Opening %s\n", sd -> d_name); 
				char nextFilePath[200];
				strcpy(nextFilePath, filePath);
				strcat(nextFilePath, sd -> d_name);
				open(nextFilePath); // open the next directory
			}
			else if(strcmp(get_ext(sd -> d_name), "o") == 0) // if the file is a .o file
			{	
				printf("this is an .o file\n");
				strcat(arArgs, filePath);
				strcat(arArgs, sd -> d_name); // add it to the list of .o files in the directory
				strcat(arArgs, " ");
			}
		}
	
	}
	
	printf("Executing: %s\n", arArgs); // display the entire command to create the library. 	
	err = system(arArgs); // create the .a file
	if(err != 0)
	{
		char str[511];
		setbuf(stderr, str);
		printf("Error: %s\nerr = %d\n", str, err);
	}
	
	char mvArgs[255];
	strcpy(mvArgs, "cp ");
	strcat(mvArgs, strcat(fileName, ".a "));
	strcat(mvArgs, strcat(name, "/.."));
	err = system(mvArgs);
	if(err != 0)
	{
		char str[511];
		setbuf(stderr, str);
		printf("Error: %s\nerr = %d\n", str, err);
	}
	//"mv ", "source", "new location", NULL};
	
	
	
	
	
	printf("\nExiting this directory\n");
	closedir(dir); // close directory
}

// compile the .a and .o files into an executable
void compile(char name[])
{
	int err = 0;
	// this command has unknown arguments so it uses the same place variable to place the NULL terminator. 
	char cmpArgs[255];
	strcpy(cmpArgs, "gcc ");
	
	DIR *dir;
	struct dirent *sd;
	
	
	
	
	dir = opendir(name);
	while((sd=readdir(dir)) != NULL)
	{
		if(strcmp(get_ext(sd -> d_name), "o") == 0)
		{
			char cpyArgs[255];
			strcpy(cpyArgs, "cp ");
			strcat(cpyArgs, name);
			strcat(cpyArgs, "/");
			strcat(cpyArgs, sd -> d_name);
			strcat(cpyArgs, " .");
			printf("cpyArgs = %s\n", cpyArgs);
			err = system(cpyArgs);
			if(err != 0)
			{
				char str[511];
				setbuf(stderr, str);
				printf("Error: %s\nerr = %d\n", str, err);
			}
			
		}
	}
	
	
	
	closedir(dir);
	
	
	
	
	dir = opendir("."); // open directory that contains the current executable
	
	printf("Starting Linking Process\n");
	while((sd=readdir(dir)) != NULL) // goes through files finding all .a and .o files. 
		{
			printf("Name is %s\n", sd -> d_name);
			if(strcmp(get_ext(sd -> d_name), "o") == 0) // if it finds a .o file
			{
					printf("hit .o\n");
					strcat(cmpArgs, sd -> d_name); // adds the .o file to the list of arguments
					strcat(cmpArgs, " ");
			}
			else if(strcmp(get_ext(sd -> d_name), "a") == 0) // if it finds a .a file
			{
					printf("hit .a\n");
					strcat(cmpArgs, sd -> d_name); // adds the .a file to the list of arguments
					strcat(cmpArgs, " ");
			}
		}
		

		// adds the rest of the arguments
		strcat(cmpArgs, "-o ");

		printf("Name = %s\n", name);
		strcat(cmpArgs, name);
		strcat(cmpArgs, "Output");
		
		// displays the arguments
		printf("cmpArgs = %s\n", cmpArgs);

		
		
		err = system(cmpArgs); // compiles the file. 
		if(err != 0)
		{
			char str[511];
			setbuf(stderr, str);
			printf("Error: %s\nerr = %d\n", str, err);
		}
		
		printf("\nDone Compiling!\n");
	closedir(dir);
}

void cleanup(char name[])
{
	
	int err = 0;
	
	
	//sleep(1);
	char rmArgs[255] = "rm ";
	
	DIR *dir;
	struct dirent *sd;
	
	dir = opendir(".");
	
	printf("Starting Linking Process\n");
	while((sd=readdir(dir)) != NULL)
		{
			printf("Name is %s\n", sd -> d_name);
			if((strcmp(get_ext(sd -> d_name), "o") == 0) || (strcmp(get_ext(sd -> d_name), "a") == 0))
			{
				printf("%s IS TO BE DELETED\n", sd -> d_name);
				strcat(rmArgs, sd -> d_name);
				
				
				err = system(rmArgs);
				if(err != 0)
				{
					char str[511];
					setbuf(stderr, str);
					printf("Error: %s\nerr = %d\n", str, err);
				}	
				printf("rm args = %s\n", rmArgs);
				strcpy(rmArgs, "rm ");
			}
		}	
}




void remove_backslash(char* in)
{
	char *str = in;
	for(int i = 0; i < strlen(str); i++)
		if((str[i] == '/') || (str[i] == '\\'))
			str[i] = '-';
		
}



