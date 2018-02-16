#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <chrono>
#include "websocket.h"

using namespace std;

webSocket server;

// DEFINE PONG VALUES HERE.
struct pongPaddle {
	// pongPaddle should be complete.
	int x_Pos;
	int y_Pos;
	int width;
	int height;
	int x_Speed;
	int y_Speed;
	const char* orientation;
	int score;

	void updatePosition(string message) {
		// There are only 2 ways a player can be oriented, make sure the orientations are only that.
		if (orientation == "HORIZONTAL") {
			// There are three messages a player can send: Positive Move (RIGHT and DOWN), Negative Move (LEFT and UP), and STILL.
			if (message == "RIGHT") {
				x_Speed = 4;
				if ((x_Pos + width)+ x_Speed > 700) {
					x_Speed = 0;
					x_Pos = 700 - width;
				}
				else {
					x_Pos += x_Speed;
				}
			}
			else if (message == "LEFT"){
				x_Speed = -4;
				if (x_Pos + x_Speed < 0) {
					x_Speed = 0;
					x_Pos = 0;
				}
				else {
					x_Pos += x_Speed;
				}
			}
			else if (message == "STOP") {
				x_Speed = 0;
			}
		}
		else if (orientation == "VERTICAL") {
			if (message == "UP") {
				y_Speed = -4;
				if (y_Pos + y_Speed < 0) {
					y_Speed = 0;
					y_Pos = 0;
				}
				else {
					y_Pos += y_Speed;
				}
			}
			else if (message == "DOWN"){
				y_Speed = 4;
				if ((y_Pos + height) + y_Speed > 700) {
					y_Speed = 0;
					y_Pos = 700 - height;
				}
				else {
					y_Pos += y_Speed;
				}
			}
			else if (message == "STOP") {
				y_Speed = 0;
			}
		}
	}
};


struct pongBall {
	int x_Pos;
	int y_Pos;
	int radius;
	int x_Speed;
	int y_Speed;

	void moveBall(pongPaddle& player) {
		x_Pos += x_Speed;
		y_Pos += y_Speed;
		
		int topX = x_Pos - 5;
		int topY = y_Pos - 5;
		int bottomX = x_Pos + 5;
		int bottomY = y_Pos + 5;

		if (bottomX > 700) {
			x_Pos = 695;
			x_Speed = -x_Speed;
		}
		else if (topX < 0) {
			x_Pos = 5;
			x_Speed = -x_Speed;
		}
		else if (bottomY > 700) {
			y_Pos = 695;
			y_Speed = -y_Speed;
			player.score = 0;			// Hard coded the player score reset.
		}
		else if (topY < 0) {
			y_Pos = 5;
			y_Speed = -y_Speed;
		}

		if (topY < (player.y_Pos + player.height) && bottomY > player.y_Pos && topX < (player.x_Pos + player.width) && bottomX > player.x_Pos) {
			y_Speed = -3;
			x_Speed += (player.x_Speed / 2);
			y_Pos += y_Speed;
			player.score++;
		}
	}

	void startGame() {
		y_Speed = 3;
	}
};

pongPaddle player1{ 300, 650, 100, 10, 0, 0, "HORIZONTAL", 0 };
pongBall ball{350, 350, 6, 0, 0};
bool gameStarted = false;
// END DEFINES HERE.

/* called when a client connects */
void openHandler(int clientID){
	if (!gameStarted) {
		gameStarted = true;

		//ostringstream os;
		//os << "Player " << clientID << " has joined the game.";

		ball.startGame();

		/*
		vector<int> clientIDs = server.getClientIDs();
		for (int i = 0; i < clientIDs.size(); i++) {
			if (clientIDs[i] != clientID)
				server.wsSend(clientIDs[i], os.str());
		}
		*/
	}
	else if (gameStarted) {
		server.wsClose(clientID);
	}
    //server.wsSend(clientID, "Welcome!");
}

/* called when a client disconnects */
void closeHandler(int clientID){
    /*
	ostringstream os;
    os << "Player " << clientID << " has left the game.";

    vector<int> clientIDs = server.getClientIDs();
    for (int i = 0; i < clientIDs.size(); i++){
        if (clientIDs[i] != clientID)
            server.wsSend(clientIDs[i], os.str());
    }
	*/
}

/* called when a client sends a message to the server */
void messageHandler(int clientID, string message){
    //ostringstream os;
    //os << "Stranger " << clientID << " says: " << message;

	player1.updatePosition(message);

	/*
    vector<int> clientIDs = server.getClientIDs();
    for (int i = 0; i < clientIDs.size(); i++){
        if (clientIDs[i] != clientID)
            server.wsSend(clientIDs[i], os.str());
    }
	*/
	
}

/* called once per select() loop */
void periodicHandler(){	
	// variable refresh is in nanoseconds instead of milliseconds because it was easier to make it work with
	// my understanding of the Chrono library. The amount of nanoseconds in refresh will need to be changed 
	//to accommodate the time spent on the calculating game state, resulting in ~60 packets getting sent to
	//the client every second.
	chrono::nanoseconds refresh{ 10000000 };
	static chrono::steady_clock::time_point refreshTime = chrono::steady_clock::now();
	chrono::steady_clock::time_point currentTime = chrono::steady_clock::now();

	if (chrono::nanoseconds{currentTime-refreshTime}.count() >= refresh.count()) {
		ball.moveBall(player1);

		ostringstream os;
		os << ball.x_Pos << "|" << ball.y_Pos << "|" << player1.x_Pos << '|' << player1.y_Pos << '|' << player1.score;

		vector<int> clientIDs = server.getClientIDs();
		for (int i = 0; i < clientIDs.size(); i++)
			server.wsSend(clientIDs[i], os.str());

		refreshTime = chrono::steady_clock::now();
	}
	
}

int main(int argc, char *argv[]){
    int port = 8000;
	
    /* set event handler */
    server.setOpenHandler(openHandler);
    server.setCloseHandler(closeHandler);
    server.setMessageHandler(messageHandler);
    server.setPeriodicHandler(periodicHandler);

    /* start the chatroom server, listen to ip '127.0.0.1' and port '8000' */
    server.startServer(port);

    return 1;
}
