/********************************************************************************
  Creates a series of files that hold descriptions of the in-game rooms and how
  rooms are connected
*********************************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>	// Needed for Process ID
#include <stdbool.h>
#include <string.h>

// Global Variables
int NUM_ROOMS = 7;

// Definition for Room struct
struct Room
{
	char name[9];                        // Max of 8 characters + null terminator
	char type[12]; 	                     // START_ROOM, END_ROOM, MID_ROOM
	int numConnections;                  // Must be between 3 and 6
	struct Room* outboundConnections[6]; // Declare max for struct size
};

// Function Declarations
void initializeRooms(struct Room*);
bool IsGraphFull(struct Room*);
struct Room *GetRandomRoom(struct Room*);
bool ConnectionAlreadyExists(struct Room*, struct Room*);
bool CanAddConnectionFrom(struct Room*);
bool IsSameRoom(struct Room*, struct Room*);
void ConnectRoom(struct Room*, struct Room*);
void AddRandomConnection(struct Room*);
void displayRooms(struct Room*);
void createRoomFiles(struct Room*);
void createDirectoryAndFiles(struct Room*);

/***************************************************************************************
* Main Function
****************************************************************************************/
int main(void)
{
	// Seed random
	srand(time(NULL));


	// roomArray will have 7 elements
	struct Room roomArray[NUM_ROOMS];

	// Initializes the room structs
	initializeRooms(roomArray);

	// Create all connections in graph
	while (IsGraphFull(roomArray) == false)
	{
                AddRandomConnection(roomArray);
	}

	// Creates directory and room files
	createDirectoryAndFiles(roomArray);

	return 0;
}


/***************************************************************************************
* Creates 7 room structs with random order and room types
****************************************************************************************/
void initializeRooms(struct Room *roomArray)
{
	// Hard coded string array of room names
	char *roomNameList[10] = {"Lion", "Wolf", "Kraken", "Dragon", "Stag", "Hawk", "Dog", "Bear", "Crow", "Trout"};

	// Shuffling pointers more efficient than using strcpy
	int x;
	char *temp;	// Used to swap elements
	int randIndex, randIndex2; // Both randomnly set to shuffle array

	// Does 50 random shuffles, appears to work well at randomizing order
	for(x = 0; x < 50; x++)
	{
		// Generates random integer between 0 and 9
		randIndex  = rand() % 10;	
		randIndex2 = rand() % 10;
		// Swaps char array pointers
		temp = roomNameList[randIndex];
		roomNameList[randIndex] = roomNameList[randIndex2];
		roomNameList[randIndex2] = temp;
	}

	// After shuffling take first 7 as rooms to be used
	
	// Initialize first room as START
	strcpy(roomArray[0].name, roomNameList[0]);
	strcpy(roomArray[0].type, "START_ROOM");
	roomArray[0].numConnections = 0;

	// Initialize second room as END
	strcpy(roomArray[1].name, roomNameList[1]);
	strcpy(roomArray[1].type, "END_ROOM");
	roomArray[1].numConnections = 0;

	// Initialize the rest as MID
	for(x = 2; x < NUM_ROOMS; x++)
	{
		strcpy(roomArray[x].name, roomNameList[x]);
		strcpy(roomArray[x].type, "MID_ROOM");
		roomArray[x].numConnections = 0;
	}
}

/***************************************************************************************
* Boolean to determine if graph is full, returns true if it is full
****************************************************************************************/
bool IsGraphFull(struct Room *roomArray)  
{
 	int x;
	// Loop through every element of array, return false if any have incorrect number
	for(x = 0; x < NUM_ROOMS; x++) 
	{
		if(roomArray[x].numConnections < 3 || roomArray[x].numConnections > 6)
			return false;
	}
	// Return true if all rooms have appropriate number of connections
	return true;
}

/***************************************************************************************
* Returns random room from roomArray, does not validate 
****************************************************************************************/
struct Room *GetRandomRoom(struct Room *roomArray)
{
	// Generates random integer between 0 and 6
	int randIndex  = rand() % NUM_ROOMS;	
	return &roomArray[randIndex];	
}

/***************************************************************************************
* Returnes true if connection already exists between two rooms
****************************************************************************************/
// Returns true if a connection from Room x to Room y already exists, false otherwise
bool ConnectionAlreadyExists(struct Room *x, struct Room *y)
{
	int i;
	// 6 Elements in outboundConnections array
	for(i = 0; i < x->numConnections; i++)
	{
		// If any of outbound connections shares name with other passed in Room
		if(strcmp(x->outboundConnections[i]->name, y->name) == 0)
		{
			return true;
		}
	}	
	// If not matches return false
	return false;
}

/***************************************************************************************
* Returns true if room has less than 6 outbound connections
****************************************************************************************/
bool CanAddConnectionFrom(struct Room *x) 
{
	// If less than 6 connections, room can still be added
	if(x->numConnections < 6)
		return true;
	// If 6 connections, no more rooms can be added
	return false;
}

/***************************************************************************************
* Returns true if two rooms have the same name (are same room)
****************************************************************************************/
bool IsSameRoom(struct Room *x, struct Room *y) 
{
	// If the names are the same, they are same room
	// strcmp returns 0 if theyre same
	if(strcmp(x->name, y->name) == 0)
	{
		return true;
	}
	// If not, different rooms
	return false;
}

/***************************************************************************************
* Connects both rooms to each other and increments counts
****************************************************************************************/
void ConnectRoom(struct Room *x, struct Room *y) 
{
	// Adds y as an outbound connection of x and increments
	x->outboundConnections[x->numConnections] = y;
	x->numConnections++;
	// Vice versa
	y->outboundConnections[y->numConnections] = x;
	y->numConnections++;
}


/***************************************************************************************
* Picks two random rooms, then connects them if valid
****************************************************************************************/
void AddRandomConnection(struct Room *roomArray)  
{
	// Pointers to Rooms
        struct Room *A;  
        struct Room *B;

	// GETTING STUCK IN THIS LOOP

	// Perform Loop until CanAddConnectionFrom A and B, Connection doesnt exist, and different rooms
        do
        {
		A = GetRandomRoom(roomArray);
  	        B = GetRandomRoom(roomArray);
        }
        while(CanAddConnectionFrom(A) == false || CanAddConnectionFrom(B) == false ||
	      IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

        ConnectRoom(A, B);  
       
	 // Shouldnt need this line, I wrote it to go both ways
        //ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates
}

/***************************************************************************************
* Diplays rooms and information about them, for troubleshooting
****************************************************************************************/
void displayRooms(struct Room *roomArray)
{
	int x;
	for(x = 0; x < NUM_ROOMS; x++)
	{
		printf("Name: %s\tType: %s\tNumber of Connections: %i\n", 
		       roomArray[x].name, roomArray[x].type, roomArray[x].numConnections);
	}
}

/***************************************************************************************
* Creates directories, room files, and outputs information to them
****************************************************************************************/
void createDirectoryAndFiles(struct Room *roomArray)
{
	// CREATE DIRECTORY
	
	// Gets process ID of program
	pid_t PID = getpid();

	// String that will hold directory name and file names
	char directoryName[20];
	char fileName[30];  // Needs to be longer (holds directory name)

	// Initializes array to null terminators
	memset(directoryName, '\0', 20);

	// Concatenates kuskc.rooms. with the process ID
	sprintf(directoryName, "kuskc.rooms.%d", PID);

	// Creates directory
	int makeDirectory = mkdir(directoryName, 0755);

	// CREATE FILES

	// Loop through every element of array, making file for each
	int x;
	for(x = 0; x < NUM_ROOMS; x++)
	{
		// Resets fileName everytime through
		memset(fileName, '\0', 30);
		// Creates file name like /directoryName/fileName
		sprintf(fileName, "%s/%s", directoryName, roomArray[x].name);

	
		FILE* myFile = fopen(fileName, "w");	
	
		// Output room name
		fprintf(myFile, "ROOM NAME: %s\n", roomArray[x].name);
		
		// Loop through every connection
		int i;
		for(i = 0; i < roomArray[x].numConnections; i++)
		{
			// Output every outbound connection room name
			fprintf(myFile, "CONNECTION %i: %s\n", i+1, 
                                roomArray[x].outboundConnections[i]->name);
		}

		// Output room type
		fprintf(myFile, "ROOM TYPE: %s\n", roomArray[x].type);
		
		fclose(myFile);
	}
}
