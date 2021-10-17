#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char *nameOne[256]; // player 1's name
char *nameTwo[256]; // player 2's name
int loopState = 1; // controls the game loop
int winStatus = 0; // tells if someone has won, 0 if noones won, 1 if someones won
int winPlayer = 0; // tells which player has won
int turn = 1; // which players turn it is Player 1 is 1, player 2 is -1
int **board; // connect 4 board I found on google images was 6x7
int width = 7; // the width of the board
int height = 7; // the height of the board + 1
int side = 0; // 0 for client side, 1 for server side. 
int sockfd , newsockfd;

// functions
int setup(); // sets up the game also allocates board
int namePrint(); // prints out the names of the players (this was originally due to testing but I thought it looked nice
int teardown(int); // prints DESTROYING THE GAME also frees board
char getInput(int); // gets input, int is what player needs input (whos turn it is)
int update(char); // updates with the input given (char)
int display(); // shows the board eventually rn just shows whats updated
void displayBoard(); // displays the board
int checkWin(); // checks if a player has won
int boardFull(); // checks if the board is full





int main(int argc, char *argv[])															
{
	int round = 0;																			// set round number
	setup(); 																				// allocate memeory for the board, set up temp names
	
	
	
	if(argc == 2) // SERVER SIDE
	{
		side = 1; 																			// says we're on the server
		
		int portNum; 																		// integers for networking
		int err; 																			// integer for error checking
		char buffer[255]; 																	// array for input
		
		struct sockaddr_in serv_addr , client_addr;
		socklen_t clientlen;
		
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if(sockfd < 0)
		{
			printf("Error opening Socket\n");
		}
		
		
		portNum = atoi(argv[1]);
		
		serv_addr.sin_family = AF_INET;														// set family
		serv_addr.sin_addr.s_addr = INADDR_ANY;												// set address, in this case 0
		serv_addr.sin_port = htons(portNum);												// set port, I've been using 9003
		
		if(bind(sockfd, (struct sockaddr *) &serv_addr , sizeof(serv_addr)) < 0) 			// bind to socket and error check
		{
			printf("Binding Failed\n");
			loopState = 0;
		}
		listen(sockfd, 1);																	// listen, only handles 1 client
		clientlen = sizeof(client_addr);												
		newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &clientlen);			// accept client pass to new file descriptor 
		if(newsockfd < 0)																	// error check for accept
		{
			printf("Error on Accept\n");
			loopState = 0;
		}
		while(loopState == 1)																// while game in processes
		{
			printf("round %d\n", round);													// display what round it is
																							// in hindsight I could just keep track of the round to tell when the board is full 
																							// but I didn't have that idea during project 2 and now im too scared to break anything
		
			if(round == 0)																	// name exchange for the first turn
			{
				err = read(newsockfd, nameTwo, sizeof(nameTwo));							// read in and set err for error testing
				if(err < 0)																	// read error testing
					printf("Error on reading Name");
				printf("Enter name:\n");													// get players name
				scanf("%[^\n]%*c", nameOne);
				err = write(newsockfd, nameOne, strlen(nameOne));							// write name and set err for error testing
				if(err < 0)																	// write error testing
					printf("Error on Writing Name\n");
				
				
			}
			else																			// gameplay after first turn is done
			{
				err = read(newsockfd, buffer, sizeof(buffer));								// read in and set err for error testing
				if(err < 0)																	// read error testing
					printf("Error on reading\n");
				
				update(buffer[0]);															// update based on read information

				printf("Other player choose : %c\n", buffer[0]);							// display other players move
																							// display the new board
				display();
				
				if(winStatus == 0)															// if the last read in move did not win the game
				{
					
					turn*=-1;																// swap turns

					printf("Input now\n");
					buffer[0] = getInput(1);												// get input
					update(buffer[0]);														// update based on input
					
					display();																// display the new board
					
					turn*=-1;																// swap turns
					
					err = write(newsockfd, buffer, strlen(buffer));							// write and set err for error testing
					if(err < 0)																// write error testing
						printf("Error on Writing\n");
				}
			}
			round++;																		// increment the round
		}
			
																
	}
	
	
	
	
	
	
	else if(argc == 3) // CLIENT SIDE
	{
		
		int portNum; 																		// integer for port
		int err; 																			// integer for error checking
		struct sockaddr_in serv_addr; 														// server address
		struct hostent *server;	
		
		char buffer[255]; 																	// array for input

		portNum = atoi(argv[2]); 															// assign the port number from command line arguments
		sockfd = socket(AF_INET, SOCK_STREAM, 0); 
		if(sockfd < 0)
			printf("ERROR opening socket\n");
		
		server = gethostbyname(argv[1]);													// assign address

		
		serv_addr.sin_family = AF_INET; 													// set family
		bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length); // I wish I had realized how long this line is before I spaced these comments
		serv_addr.sin_port = htons(portNum); 												// set port, I've been using 9003
		if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) 		// connect and error check
		{
			printf("Connection failed\n");
			loopState = 0;
		}

		while(loopState == 1) 																// while the game hasn't finished
		{
			printf("round %d\n", round); 													// print round
			if(round == 0)																	// if the first round
			{

				
				printf("Enter name:\n"); // get name
				scanf("%[^\n]%*c", nameTwo);
				err = write(sockfd, nameTwo, strlen(nameTwo));								// write name and set err for error testing
				if(err < 0)																	// write error testing
						printf("Error on Writing Name\n");
				err = read(sockfd, nameOne, sizeof(nameOne));								// read name and set err for error testing
				if(err < 0)																	// read error testing
					printf("Error on reading Name\n");
				
			
				
				
			}
			else																			// actions for all turns after the first
			{
				display(); 																	// displays board
				
				printf("Input now\n");
				buffer[0] = getInput(-1); 													// sends the first character entered to the getInput method
				update(buffer[0]); 															// update the state of the world using the first character entered

				display(); 																	// display the board

				turn*=-1; 																	// change who's turn it is
					
				err = write(sockfd, buffer, strlen(buffer)); 								// write to the server what was done
				if(err < 0) 
					printf("Error on writing\n");

					
				err = read(sockfd, buffer, 255);									 		// get info from server of what they did
				if(err < 0)
					printf("Error on reading\n");
				if(winStatus == 0)	
				{
					update(buffer[0]); 														// update the game based on that info
					printf("Other player chose: %c\n", buffer[0]); 							// print what choice the other player made
				}
				turn*=-1; 																	// swap turns
						
				
					
			
			}	
			
			round++; 																		// increment the round

			
		}

		if(winStatus != 0)																	
			if(winPlayer == -1)
				display();
	}

	
	teardown(argc); 																		// call teardown
	
	
	return 0;
}




int setup() 
{
	board = malloc(sizeof(int*)*height);
	for(int i = 0; i < height; i++)
		board[i] = malloc(sizeof(int) * width);
	for(int i = 0; i < width; i++)
		board[height-1][i] = 3;	
	printf("Setting up the game! \n");
	/**
	printf("\n Player 1 Enter Name: \n");
	scanf("%s", nameOne);
	printf("\n Player 2 Enter Name: \n");
	scanf("%s", nameTwo);
	//displayBoard();
	**/
	
	strcpy(nameOne, "a");
	strcpy(nameTwo, "b");
	return 0;
}



int namePrint() // startup sequence, makes sure everyone knows who they are
{
	printf("Player 1 is: ");
	printf("%s", nameOne);
	printf("\nPlayer 2 is: ");
	printf("%s", nameTwo);
	printf("\n");
}



int teardown(int argc) // frees memory when game is over
{
	printf("DESTROYING THE GAME\n\n");
	free(board);
	if(argc == 2)
	{															
		close(sockfd);	
		close(newsockfd);
	}
	else if(argc == 3)
	{
		close(sockfd);
	}
}



char getInput(int player) // valid inputs are A-G and a letter for quitting the game
{
	int validInput = 0;
	char input;
	while(validInput == 0)
	{
		if(player == 1)
			printf("%s please input your row (A-G)\n", nameOne);
		else
			printf("%s please input your row (A-G)\n", nameTwo);
		
		printf("Enter J to exit\n");
		
		scanf("%s", &input);
		printf("\n");
		
		if(((input >= 'A') && (input <= 'G') || (input == 'J')) && (board[0][input-65] == 0))
			validInput = 1;
		else
			printf("\nInvalid input, please try again\n");
		
		printf("\n");
		
	}
	return input;
}


// updates the game to place pieces where they should go or check if a player has won
// also checks if board is full
int update(char input) 
{
	int loop = 1;
	int i = 0;
	
	if(input == 'J')
	{
		printf("You've printed J\n");
		loopState = 0;
	}
	else
		while(loop)
		{
			if(board[i][input-65] != 0)// minus 65 from letter input array input is y,x (down then right)
			{
				board[i-1][input-65] = turn;
				loop = 0;
			}
			i++;
		}
	checkWin();
	if(boardFull())
		loopState = 0;

}
// checks if the board is full
int boardFull()
{
	int full = 1;
	for(int i = 0; i < height; i++)
		for(int j = 0; j < width; j++)
			if(board[i][j] == 0)
				full = 0;
	return full;
	
}

// checks if a player has placed 4 pieces next to eachother in any direction

/*
	Uses 4 flags to make sure the check does not go out of bounds
	if its at least 4 spaces in any direction its flag is set to true
	Checks are divided into two main groups involving the vertical flags
	Each group has three statements for vertical left, vertical right, and vertical.
	After the groups are the two horizontal checks
*/
int checkWin()
{
	int clearUp = 0;
	int clearDown = 0;
	int clearLeft = 0;
	int clearRight = 0;
	for(int i = 0; i < height-1; i++)
		for(int j = 0; j < width; j++)
			if(board[i][j] != 0)
			{
				if(i >= 3)
					clearUp = 1;
				if(i <= 1)
					clearDown = 1;
				if(j >= 3)
					clearLeft = 1;
				if(j <=2)
					clearRight = 1;
				
				if(clearUp)
				{
					if(clearLeft)
						if((board[i-1][j-1] == board[i][j]) && (board[i-2][j-2] == board[i][j]) && (board[i-3][j-3] == board[i][j]))
						{
							printf("\nA\n");
							winPlayer = turn;
						}
					if(clearRight)
						if((board[i-1][j+1] == board[i][j]) && (board[i-2][j+2] == board[i][j]) && (board[i-3][j+3] == board[i][j]))
						{
							printf("\nB\n");
							winPlayer = turn;
						}
					if((board[i-1][j] == board[i][j]) && (board[i-2][j] == board[i][j]) && (board[i-3][j] == board[i][j]))
					{
						printf("\nC\n");
						winPlayer = turn;
					}
				}
				
				if(clearDown)
				{
					if(clearLeft)
						if((board[i+1][j-1] == board[i][j]) && (board[i+2][j-2] == board[i][j]) && (board[i+3][j-3] == board[i][j]))
							winPlayer = turn;
					else if(clearRight)
						if((board[i+1][j+1] == board[i][j]) && (board[i+2][j+2] == board[i][j]) && (board[i+3][j+3] == board[i][j]))
							winPlayer = turn;
					else
						if((board[i+1][j] == board[i][j]) && (board[i+2][j] == board[i][j]) && (board[i+3][j] == board[i][j]))
							winPlayer = turn;
				}
				if(clearLeft)
					if((board[i][j-1] == board[i][j]) && (board[i][j-2] == board[i][j]) && (board[i][j-3] == board[i][j]))
							winPlayer = turn;
				if(clearRight)
					if((board[i][j+1] == board[i][j]) && (board[i][j+2] == board[i][j]) && (board[i][j+3] == board[i][j]))
							winPlayer = turn;
				
					
					
				clearUp=0;
				clearDown=0;
				clearLeft=0;
				clearRight=0;
				
				
			}
	
	
	if(winPlayer !=0)
	{
		displayBoard();
		printf("A Player Has Won\n");
		winStatus = 1;
		loopState = 0;
	}
	return 0;
}




/*
	Chooses display based on game state
*/
int display()
{
	
	
	if(winStatus == 1)
	{
		printf("\n\n-----------------------------\n\n\n");
		//if(side == 1)
			//turn*=-1;
		if(winPlayer == -1)
			printf("%s has WON!!!\n", nameOne);
		else
			printf("%s has WON!!!\n", nameTwo);
		printf("\n\n-----------------------------\n\n");
		loopState = 0; 
	}
	else
		displayBoard();
	
}


// displays the board. 
void displayBoard()
{
	
	printf("   A B C D E F G\n");
	for(int i = 0; i < 6; i++)
	{
		printf("  | ");
		for(int j = 0; j < 7; j++)
		{
		if(board[i][j] == 0)
			printf("- ");
		else if(board[i][j] == 1)
			printf("x ");
		else
			printf("o ");
		}
		printf("|\n");
	}
	printf("\n");
	
}
