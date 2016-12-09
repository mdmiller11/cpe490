// Server.cpp (for tcp)
// Compile and link with wsock32.lib
// Usage: Server PortNumber
// Example: DServer 2000
// a file called recfile is created

#include <stdio.h>
#include <winsock.h>
#include <dshow.h>
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;


// Function prototype
void StreamServer(short nPort);

// Helper macro for displaying errors
#define PRINTERROR(s)	\
		fprintf(stderr,"\n%: %d\n", s, WSAGetLastError())


int main(int argc, char **argv)
{	

	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	int nRet;
	short nPort;

	// Check for port argument
	if (argc != 2)	{
		fprintf(stderr,"\nSyntax: server PortNumber\n");
		return 0;
	}


	// get port no
	nPort = atoi(argv[1]);


	// Initialize WinSock and check version
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)	{	
		fprintf(stderr,"\n Wrong version\n");
		return 0;
	}

	// perform file transfer with client
	StreamServer(nPort);

	// Release WinSock
	WSACleanup();
}


void StreamServer(short nPort)
{
	// Create a TCP/IP stream socket to "listen" with
	SOCKET	listenSocket;

	listenSocket = socket(AF_INET,			// Address family
						  SOCK_STREAM,		// Socket type
						  IPPROTO_TCP);		// Protocol
	if (listenSocket == INVALID_SOCKET)	{
		PRINTERROR("socket()");
		return;
	}

	// Fill in the address structure
	SOCKADDR_IN saServer;		

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock supply address
	saServer.sin_port = htons(nPort);		// Use port from command line

	// bind the name to the socket
	int nRet;

	nRet = bind(listenSocket,				// Socket 
				(LPSOCKADDR)&saServer,		// Our address
				sizeof(struct sockaddr));	// Size of address structure
	if (nRet == SOCKET_ERROR) {
		PRINTERROR("bind()");
		closesocket(listenSocket);
		return;
	}

	int nLen;
	nLen = sizeof(SOCKADDR);
	char szBuf[2048];

	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR)	{
		PRINTERROR("gethostname()");
		closesocket(listenSocket);
		return;
	}

	// Show the server name and port number
	printf("\nServer named %s waiting on port %d\n",
			szBuf, nPort);

	// Set the socket to listen
	
	nRet = listen(listenSocket,					// Bound socket
				  SOMAXCONN);					// Number of connection request queue
	if (nRet == SOCKET_ERROR) {
		PRINTERROR("listen()");
		closesocket(listenSocket);
		return;
	}

	namedWindow("MyVideoServer",CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"


	// Wait for an incoming request
	SOCKET	remoteSocket;
	while(1) {
	remoteSocket = accept(listenSocket,			// Listening socket
						  NULL,					// Optional client address
						  NULL);
	if (remoteSocket == INVALID_SOCKET)	{
		PRINTERROR("accept()");
		closesocket(listenSocket);
		return;
	}

 
   double dWidth = 640; //get the width of frames of the video
   double dHeight = 480; //get the height of frames of the video

   cout << "Frame Size = " << dWidth << "x" << dHeight << endl;

   Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));

	
	//while(1)
	//{

	printf("Wait data from client\n");

	int c, i, j=1;
	long totbyte=0;
	FILE *nfp;	
	if ((nfp = fopen("recfile", "w+b")) ==  NULL) {
			printf("Error: Can't create file: 'recfile'\n");
			return;
	}
	
	// Receive data from the client
	j=1;
	while (j>0)	{
		memset(szBuf, 0, sizeof(szBuf));		// clear buffer
		
		nRet = recv(remoteSocket,				// Connected client
				szBuf,							// Receive buffer
				sizeof(szBuf),					// Lenght of buffer
				0);								// Flags

		j=nRet;

		if (nRet == INVALID_SOCKET)	{
			printf("\nTotal bytes received: %.f\n", (float)totbyte);
			PRINTERROR("recv()");
			fclose(nfp);
			closesocket(listenSocket);
			closesocket(remoteSocket);
			return;
		}
		i=0;
		// receive data from client and save to file
		while (nRet > i) {
			c=szBuf[i];
			i++;
			totbyte++;
			putc(c, nfp);
		}
		printf(".");

		Mat frame;
		FILE * frameFile;
		if((frameFile = fopen("recfile", "rb"))!=NULL){
		fseek(frameFile, 0, SEEK_END);
		long lSize = ftell(frameFile);
		rewind (frameFile);

	  // allocate memory to contain the whole file:
	  char* buffer = (char*) malloc (sizeof(char)*lSize);
	  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

	  // copy the file into the buffer:
	  frame = fread (buffer,1,lSize,frameFile);

	  if(!frame.empty()){
		imshow("MyVideoServer", frame); //show the frame in "MyVideo" window
		cout << "Frame not empty" << endl;
	  }
	  else {cout << "Frame empty" << endl;}
		}
		//else{cout << "file null" << endl;}
        if (waitKey(10) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       {
            cout << "esc key is pressed by user" << endl;
            break; 
       }
	} 
	fclose(nfp);

	}

	//printf("\nTotal bytes received: %.f\nNew File 'recfile' is created.\n", (float)totbyte);
	
	// close file 
	// Close BOTH sockets before exiting
	closesocket(remoteSocket);
	closesocket(listenSocket);
	return;
}
