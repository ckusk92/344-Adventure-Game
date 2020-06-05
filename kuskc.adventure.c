/********************************************************************************
  Provides interface for playing the game using the most recently generated rooms
*********************************************************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>	// Needed for Process ID
#include <stdbool.h>
#include <string.h>
#include <dirent.h>     // Needed to find directory
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

// Global Variables
int NUM_ROOMS = 7;
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;


// Definition for Room struct
struct Room
{
	char name[9];                        // Max of 8 characters + null terminator
	char type[12]; 	                     // START_ROOM, END_ROOM, MID_ROOM
	int numConnections;                  // Must be between 3 and 6
	struct Room* outboundConnections[6]; // Declare max for struct size
	char connectionArray[6][10];	     // String array of connection names
};

// Function Declarations
void findNewestDirectory(char*);
void getFileNames(char[NUM_ROOMS][40]);
void buildRoomArray(struct Room*);
void setConnections(struct Room*);
void playGame(struct Room*);
void* writeTime(void*);
void readTime();

/***************************************************************************************
* Main Function
****************************************************************************************/
int main(void)
{
	struct Room roomArray[NUM_ROOMS];
	buildRoomArray(roomArray);        // Calls getFileNames and setConnections
	playGame(roomArray);

	// Clean up mutex
	pthread_mutex_destroy(&my_mutex);
	
	return 0;
}


/***************************************************************************************
* Finds name of newest directory and setes directoryName pointer to it 
****************************************************************************************/
void findNewestDirectory(char *directoryName)
{
	// Largely taken from 2.4 from Canvas
	int newestDirTime = -1; // Modified timestamp of newest subdir examined
        char targetDirPrefix[32] = "kuskc.rooms."; // Prefix we're looking for
        char newestDirName[30]; // Holds the name of the newest dir that contains prefix
        memset(newestDirName, '\0', sizeof(newestDirName));

	DIR* dirToCheck; // Holds the directory we're starting in
	struct dirent *fileInDir; // Holds the current subdir of the starting dir
	struct stat dirAttributes; // Holds information we've gained about subdir

	dirToCheck = opendir("."); // Open up the directory this program was run in

	if (dirToCheck > 0) // Make sure the current directory could be opened
	{
  		while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
  		{
	    		if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
    			{
       			 	stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

        			if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
        			{
          				newestDirTime = (int)dirAttributes.st_mtime;
          				memset(newestDirName, '\0', sizeof(newestDirName));
          				strcpy(newestDirName, fileInDir->d_name);
    	       	                 }
         	        }
                }   
        }    

	closedir(dirToCheck); // Close the directory we opened

	// Copies string of newest directory into directoryName pointer
	strcpy(directoryName, newestDirName);
}



void getFileNames(char fileNames[NUM_ROOMS][40])
{
	// Set directoryName variable and memset it to null terminators
	char directoryName[256];
	memset(directoryName, '\0', 256);	

	// Open the most recently created directory
	findNewestDirectory(directoryName);

	// SEE IF ANY VARIABLES BELOW CAN BE DELETED

	// Need a way to read in every file within directory
	DIR* FD;	// Directory pointer
        struct dirent* fileIn;

	FD = opendir(directoryName);

	int x = 0;
	
	// While files in directory can be read
        while ((fileIn = readdir(FD)) && x < 7)  
        {
		// Leave loop if trying to go into parent directory
        	if (!strcmp (fileIn->d_name, "."))
            		continue;
        	if (!strcmp (fileIn->d_name, ".."))    
            		continue;
		sprintf(fileNames[x], "%s/%s", directoryName, fileIn->d_name);

		// Increment x every time in loop
		x++; 
	}

	// Close directory when done
	closedir(FD);
}

/***************************************************************************************
* Reads in data from room files and creates roomArray 
****************************************************************************************/
void buildRoomArray(struct Room *roomArray)
{
	FILE *fp;
	int x;

	// Read each file to its respective element in Room array
	char fileNames[NUM_ROOMS][40];
	for(x = 0; x < NUM_ROOMS; x++)
		memset(fileNames[x], '\0', 40);
	
	// Fill fileNames char array with correct file names 
	getFileNames(fileNames);

	char lineIn[30];
	memset(lineIn, '\0', 30);
	int numConnections;

	for(x= 0; x < 7; x++)
	{
		// Opens each file 
		fp = fopen(fileNames[x], "r");
	
		// initializes room name before other properties
		fgets(lineIn, 30, fp);
		sscanf(lineIn, "%*s %*s %s", roomArray[x].name);
	
		// Resets number of connections
		numConnections = 0;

		// While fgets can read new lines
		while(fgets(lineIn, 30, fp))
		{
			if(lineIn[0] == 'C')
			{
				// Connections rooms start with CONNECTION
				sscanf(lineIn, "%*s %*s %s", roomArray[x].connectionArray[numConnections]);
				// Increment number of connections by one after adding names
				numConnections = numConnections + 1;
				
			}
			else
			{
				// If not starting with a C will be line giving ROOM TYPE
				fgets(lineIn, 30, fp);
				sscanf(lineIn, "%*s %*s %s", roomArray[x].type);
			}	
		}
		
		roomArray[x].numConnections = numConnections;

		// Close file when done
		fclose(fp);
	}

	// Points rooms that are connected to each other
	setConnections(roomArray);
}


/***************************************************************************************
* Diplays rooms and information about them, for troubleshooting
****************************************************************************************/
void setConnections(struct Room *roomArray)
{
	// Loop through every element in room Array
	int x, y, z;
	for(x = 0; x < NUM_ROOMS; x++)
	{
		// Loop through every element in outboundConnections
		for(y = 0; y < 6; y++)
		{
			// LOOP THROUGH ALL ROOMS AGAIN?
			// If name in connectionArray is same as a Room name, connect
			for(z = 0; z < NUM_ROOMS; z++)
			{
				if(!strcmp(roomArray[x].connectionArray[y], roomArray[z].name))
				{
					roomArray[x].outboundConnections[y] = &roomArray[z];
				}
			}
		}	
	}
}

/***************************************************************************************
* Diplays rooms and information about them, for troubleshooting
****************************************************************************************/
void playGame(struct Room *roomArray)
{
	struct Room *currentRoom;	// Pointer to room user will be in 
	char userInput[10];		// Input from user, rooms have length of 8
	int numSteps = 0;		// Counts number of steps of user
	char roomsVisited[100][10];     // Array of rooms visited

	// Find starting
	int x;
	for(x = 0; x < NUM_ROOMS; x++)
		if(!strcmp(roomArray[x].type, "START_ROOM"))
			currentRoom = &roomArray[x];

	// Run game in while loop
	// strcmp will return 0 when the match
	while(strcmp(currentRoom->type, "END_ROOM"))
	{
		printf("CURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS: ", currentRoom->name);

		// Loop will go through all outboundConnections
		int x;
		for(x = 0; x < currentRoom->numConnections - 1; x++)
			printf("%s, ", currentRoom->outboundConnections[x]->name);
	
		// Print last room with a period
		printf("%s.\n", currentRoom->outboundConnections[x]->name);

		// User interface and input
		printf("WHERE TO? >");
		scanf("%s", userInput);
		printf("\n");

		if(strcmp(userInput, "time") == 0)
		{
			int resultInt;
			pthread_t time_thread;
			pthread_mutex_lock(&my_mutex);
			// Creates new thread which will write time to currentTime.txt
			resultInt = pthread_create(&time_thread,
						   NULL,
						   writeTime,
						   NULL);
			resultInt = pthread_join(time_thread, NULL);
			pthread_mutex_unlock(&my_mutex);

			// Displays time to user
			readTime();
		}
		else
		{
			// Search if room exists
			bool roomFound = false;
			for(x = 0; x < currentRoom->numConnections; x++)
			{
				// If name entered matchese one of the listed rooms
				if(strcmp(currentRoom->outboundConnections[x]->name, userInput) == 0)
				{
					// Reassign currentRoom pointer to selected room
					currentRoom = currentRoom->outboundConnections[x];
					strcpy(roomsVisited[numSteps], currentRoom->name);
					numSteps++;
					roomFound = true;
				}
			 }
			// If userInput does not match any room, output error
			if(!roomFound)
				printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
		}	
	}
	// Output message when user has found room
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n", numSteps);
	int i;
	// Prints list of rooms visited
	for(i = 0; i < numSteps; i++)
	{
		printf("%s\n", roomsVisited[i]);
	}
	
	// Take care of dangling pointers
	currentRoom = NULL;
}


/***************************************************************************************
* Writes the current time to time.txt
****************************************************************************************/
void* writeTime(void* arguments)
{
	// Looked here for info on using 
	// https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
	
	time_t currentTime;
	struct tm* info;
	char timeString[80];
	time(&currentTime);
	info = localtime(&currentTime);
	// Parses time info into desired format
	strftime(timeString, 80, "%l:%M%P, %A, %B %e, %Y", info);

	FILE *filePointer;
	// Opens file time will be written to
	filePointer = fopen("currentTime.txt", "w");
	// Prints time to file, overwriting anything that may already be there
	fprintf(filePointer, "%s\n", timeString);
	// Close file when done
	fclose(filePointer); 
}


/***************************************************************************************
* Reads the time on time.txt and outputs it to user in console
****************************************************************************************/
void readTime()
{
	FILE *filePointer;
	// Opens file time will be read from
	filePointer = fopen("currentTime.txt", "r");
	char timeString[40];
	fgets(timeString, 40, filePointer);
	printf("%s\n\n", timeString);	

	// Close file when done reading it
	fclose(filePointer);
}
