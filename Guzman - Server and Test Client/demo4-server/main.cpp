#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <chrono>
#include "websocket.h"

using namespace std;

webSocket server;

/* called when a client connects */
void openHandler(int clientID){
    ostringstream os;
    os << "Stranger " << clientID << " has joined.";

    vector<int> clientIDs = server.getClientIDs();
    for (int i = 0; i < clientIDs.size(); i++){
        if (clientIDs[i] != clientID)
            server.wsSend(clientIDs[i], os.str());
    }
    server.wsSend(clientID, "Welcome!");
}

/* called when a client disconnects */
void closeHandler(int clientID){
    ostringstream os;
    os << "Stranger " << clientID << " has leaved.";

    vector<int> clientIDs = server.getClientIDs();
    for (int i = 0; i < clientIDs.size(); i++){
        if (clientIDs[i] != clientID)
            server.wsSend(clientIDs[i], os.str());
    }
}

/* called when a client sends a message to the server */
void messageHandler(int clientID, string message){
    ostringstream os;
    os << "Stranger " << clientID << " says: " << message;

    vector<int> clientIDs = server.getClientIDs();
    for (int i = 0; i < clientIDs.size(); i++){
        if (clientIDs[i] != clientID)
            server.wsSend(clientIDs[i], os.str());
    }
}

/* called once per select() loop */
void periodicHandler(){
    static time_t next = time(NULL) + 1;
    time_t current = time(NULL);
    if (current >= next){
        ostringstream os;
		//Deprecated ctime API in Windows 10
		char timecstring[26];
		ctime_s(timecstring, sizeof(timecstring), &current);
		string timestring(timecstring);
        timestring = timestring.substr(0, timestring.size() - 1);
        os << timestring;

        vector<int> clientIDs = server.getClientIDs();
		for (int i = 0; i < clientIDs.size(); i++)
			server.wsSend(clientIDs[i], os.str());

		//cout << os.str().c_str() << endl;
        next = time(NULL) + 1;
    }
	
	// variable refresh is in nanoseconds instead of milliseconds because it was easier to make it work with
	// my understanding of the Chrono library. The amount of nanoseconds in refresh will need to be changed 
	//to accommodate the time spent on the calculating game state, resulting in ~60 packets getting sent to
	//the client every second.
	chrono::nanoseconds refresh{ 10000000 };
	static chrono::steady_clock::time_point refreshTime = chrono::steady_clock::now();
	chrono::steady_clock::time_point currentTime = chrono::steady_clock::now();

	if (chrono::nanoseconds{currentTime-refreshTime}.count() >= refresh.count()) {
		ostringstream os;
		os << "I coded this next to some latino guy and a korean girl and they were overly affectionate and that was pretty uncomfortable to me like jesus fuck anywhere else please. Also this is a test string btw, if you are seeing this in the turned-in assignment, we (Diego) made an oopsie.";
		vector<int> clientIDs = server.getClientIDs();
		for (int i = 0; i < clientIDs.size(); i++)
			server.wsSend(clientIDs[i], os.str());

		refreshTime = chrono::steady_clock::now();
	}
	
}

int main(int argc, char *argv[]){
    int port = 8000;
    
	//chrono::milliseconds refresh{ chrono::seconds{1} };		//60 fps, or as near as makes no difference.
	//cout << refresh.count() << endl;
	
	//auto refreshTime = chrono::steady_clock::now();
	//cout << refreshTime.time_since_epoch().count()  << endl;
	//cout << refreshTime.time_since_epoch().count() + refresh.count() << endl;
	//cout << (refresh.count() == refreshTime.time_since_epoch().count()) << endl;
	//auto currentTime = chrono::steady_clock::now();

	//cout << chrono::nanoseconds{currentTime-refreshTime}.count() << endl;
	
    /* set event handler */
    server.setOpenHandler(openHandler);
    server.setCloseHandler(closeHandler);
    server.setMessageHandler(messageHandler);
    server.setPeriodicHandler(periodicHandler);

	

    /* start the chatroom server, listen to ip '127.0.0.1' and port '8000' */
    server.startServer(port);

    return 1;
}
