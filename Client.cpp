// Client.cpp (for TCP)
// Compile and link with wsock32.lib
// Usage: Client ServerName PortNumber Filename
// Example: Client computer1 2000 abc.exe
// abc.exe will be transmitted to server and save as recfile

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include <winsock.h>
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

// Function prototype
void StreamClient(char *szServer, short nPort, FILE *fp);

// Helper macro for displaying errors
#define PRINTERROR(s)	\
		fprintf(stderr,"\n%: %d\n", s, WSAGetLastError())


int main(int argc, char **argv)
{


	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	int nRet;
	short nPort;
	FILE *fp;

	// Check for the host and port arguments
	if (argc != 3) {
		fprintf(stderr,"\nSyntax: Client ServerName PortNumber\n");
		return 0;
	}

	// get port no
	nPort = atoi(argv[2]);

	VideoCapture cap(0); // open the video camera no. 0

    if (!cap.isOpened())  // if not success, exit program
    {
        cout << "ERROR: Cannot open the video file" << endl;
        return -1;
    }

 namedWindow("MyVideo",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
 
   double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
   double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

   cout << "Frame Size = " << dWidth << "x" << dHeight << endl;

   Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
   //CV_FOURCC('P','I','M','1')
 VideoWriter oVideoWriter ("C:/MyVideo.avi", CV_FOURCC('M', 'J', 'P', 'G'), 20, frameSize, true); //initialize the VideoWriter object 

	//waitKey(2000);

   if ( !oVideoWriter.isOpened() ) //if not initialize the VideoWriter successfully, exit the program
   {
        cout << "ERROR: Failed to write the video" << endl;
        return -1;
   }

   while (1)
    {
        Mat frame;

        bool bSuccess = cap.read(frame); // read a new frame from video

        if (!bSuccess) //if not success, break loop
       {
             cout << "ERROR: Cannot read a frame from video file" << endl;
             break;
        }
		oVideoWriter.write(frame); //writer the frame into the file
        imshow("MyVideo", frame); //show the frame in "MyVideo" window

		FILE * frameFile;
		frameFile= fopen("frame.txt", "w");
		void* frameLoc = &frame;
		fwrite(frameLoc, sizeof(frame), 1, frameFile);

		//frameFile.open ("frame.txt");


		if ((fp = frameFile) ==  NULL) {
			printf("Error: Can't open file: %s\n", argv[3]);
			return 0;
	}

	// Initialize WinSock and check the version
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested) {	
		fprintf(stderr,"\n Wrong version\n");
		return 0;
	}

	// file transfer
	StreamClient(argv[1], nPort, frameFile);
	 namedWindow("MyVideo2",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
	
	 Mat frame2;
	 FILE* frameFile2;
	 if((frameFile2 = fopen("frame.txt", "rb"))!= NULL){
		 fseek(frameFile2, 0, SEEK_END);
		 long lSize = ftell(frameFile2);
		 rewind(frameFile2);

	char*buffer = (char*) malloc (sizeof(char)*lSize);
	if(buffer == NULL) { fputs ("Memory error", stderr); exit(2);}

	frame2 = fread (buffer, 1, lSize, frameFile2);
	if(!frame2.empty()){
		imshow("MyVideo2", frame); //show the frame in "MyVideo" window
		cout << "Frame not empty" << endl;
	  }
	  else {cout << "Frame empty" << endl;}
		}
	 
	 
	 // close file 
	fclose(fp);



        if (waitKey(10) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       {
            cout << "esc key is pressed by user" << endl;
            break; 
       }
    }

	

	// Release WinSock
	WSACleanup();
	return 0;
}


void StreamClient(char *szServer, short nPort, FILE *fp)
{

	printf("Client connecting to server: %s on port: %d",
				szServer, nPort);

	// Find the server
    LPHOSTENT lpHostEntry;

	lpHostEntry = gethostbyname(szServer);
    if (lpHostEntry == NULL) {
        PRINTERROR("gethostbyname()");
        return;
    }

	// Create a TCP/IP stream socket
	SOCKET	theSocket;

	theSocket = socket(AF_INET,				// Address family
					   SOCK_STREAM,			// Socket type
					   IPPROTO_TCP);		// Protocol
	if (theSocket == INVALID_SOCKET) {
		PRINTERROR("socket()");
		return;
	}

	// Fill in the address structure
	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
										// Server's address
	saServer.sin_port = htons(nPort);	// Port number from command line

	// connect to the server
	int nRet;

	nRet = connect(theSocket,				// Socket
				   (LPSOCKADDR)&saServer,	// Server address
				   sizeof(struct sockaddr));// Length of server address structure
	if (nRet == SOCKET_ERROR) {
		PRINTERROR("socket()");
		closesocket(theSocket);
		return;
	}

	long totbyte=0;
	char szBuf[2048];
	int c, i;
	
	printf("\n");
	do {
			i=0;
			// get file content and send to tcp channel
			while ( (i<2000) && ((c=getc(fp)) != EOF)  )	{
				szBuf[i] = c;
				i++;
				totbyte++;
			}
			
			nRet = send(theSocket,				// Connected socket
				szBuf,							// Data buffer
				i,								// Length of data
				0);								// Flags
			if (nRet == SOCKET_ERROR) {
				PRINTERROR("send()");
				closesocket(theSocket);
				return;
			}
	
			printf(".");
	} while (c != EOF);
	
	printf("\nTotal Transfered Message Size: %.0f bytes\nBye!\n", (float) totbyte);
	printf("%.0f)", (float) totbyte);
	closesocket(theSocket);
	return;
}
