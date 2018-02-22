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
	int x_Pos;
	int y_Pos;
	int width;
	int height;
	int x_Speed;
	int y_Speed;
	const char* orientation;
	int score;
	const int defaultX_Pos;
	const int defaultY_Pos;
	string playerName;
	int clientID = 9999;

	void updatePlayerName(string pName) {
		playerName = pName;
	}
	void updateClientID(int cID) {
		clientID = cID;
	}
	void resetPlayer() {
		score = 0;
		x_Pos = defaultX_Pos;
		y_Pos = defaultY_Pos;
		x_Speed = 0;
		y_Speed = 0;
	}

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
	pongPaddle** players;
	int indexOfPlayer = 4;

	void moveBall() {
		x_Pos += x_Speed;
		y_Pos += y_Speed;
		
		int topX = x_Pos - 5;
		int topY = y_Pos - 5;
		int bottomX = x_Pos + 5;
		int bottomY = y_Pos + 5;

		if (bottomX > 700) {			//Player 4
			x_Pos = 695;
			x_Speed = -x_Speed;
			//players[3]->score = 0;
			givePoint();
		}
		else if (topX < 0) {			// Player 3
			x_Pos = 5;
			x_Speed = -x_Speed;
			//players[2]->score = 0;
			givePoint();
		}
		else if (bottomY > 700) {		// Player 1
			y_Pos = 695;
			y_Speed = -y_Speed;
			//players[0]->score = 0;		
			givePoint();
		}
		else if (topY < 0) {			// Player 2
			y_Pos = 5;
			y_Speed = -y_Speed;
			//players[1]->score = 0;
			givePoint();
		}

		//if (topY < (player.y_Pos + player.height) && bottomY > player.y_Pos && topX < (player.x_Pos + player.width) && bottomX > player.x_Pos) {
		//	y_Speed = -3;
		//	x_Speed += (player.x_Speed / 2);
		//	y_Pos += y_Speed;
		//	player.score++;
		//}
		for (int i = 0; i < 4; i++) {
			if (topY < (players[i]->y_Pos + players[i]->height) && bottomY > players[i]->y_Pos && topX < (players[i]->x_Pos + players[i]->width) && bottomX > players[i]->x_Pos) {
				if (players[i]->orientation == "HORIZONTAL") {
					x_Speed += int (players[i]->x_Speed / 2);
					y_Speed = y_Speed * -1;
				}
				if (players[i]->orientation == "VERTICAL") {
					x_Speed = x_Speed * -1;
					y_Speed += int(players[i]->y_Speed / 2);
				}
				//players[i]->score++;
				indexOfPlayer = i;
			}
		}
	}

	void startGame() {
		y_Speed = 3;
	}

	void stopGame() {
		x_Speed = 0;
		y_Speed = 0;
		x_Pos = 350;
		y_Pos = 350;

		for (int i = 0; i < 4; i++) {
			players[i]->resetPlayer();
		}
	}

	void givePoint() {
		if (indexOfPlayer != 4) {
			players[indexOfPlayer]->score++;
		}
		x_Pos = 350;
		y_Pos = 350;
		if (indexOfPlayer == 0) {
			x_Speed = 3;
			y_Speed = 0;
		}
		else if (indexOfPlayer == 1) {
			x_Speed = -3;
			y_Speed = 0;
		}
		else if (indexOfPlayer == 2) {
			x_Speed = 0;
			y_Speed = 3;
		}
		else if (indexOfPlayer == 3) {
			x_Speed = 0;
			y_Speed = -3;
		}
		indexOfPlayer = 4;
	}
};
string defaultName = "NoPlayer";

pongPaddle player1{ 300, 685, 100, 10, 0, 0, "HORIZONTAL", 0, 300, 685, defaultName };
pongPaddle player2{ 300, 5, 100, 10, 0, 0, "HORIZONTAL", 0, 300, 5, defaultName };
pongPaddle player3{ 5, 300, 10, 100, 0, 0, "VERTICAL", 0, 5, 300, defaultName };
pongPaddle player4{ 685, 300, 10, 100, 0, 0, "VERTICAL", 0, 685, 300, defaultName };

pongPaddle* players[4] = { &player1, &player2, &player3, &player4 };
int playersConnected = 0;

pongBall ball{ 350, 350, 6, 0, 0, players};
bool gameStarted = false;
// END DEFINES HERE.

/* called when a client connects */
void openHandler(int clientID){
	if (playersConnected == 4) {
		server.wsClose(clientID);
	}

	for (int i = 0; i < 4; i++) {
		if (players[i]->clientID == 9999) {
			cout << "Player " << i+1 << " has joined. Client ID: " << clientID << endl;
			players[i]->updateClientID(clientID);
			playersConnected++;
			break;
		}
	}
	cout << "Players on Server: " << playersConnected << endl;
	if (playersConnected == 4 && !gameStarted) {
		gameStarted = true;
		ball.startGame();
	}

}

/* called when a client disconnects */
void closeHandler(int clientID){
	for (int i = 0; i < 4; i++) {
		if (players[i]->clientID == clientID) {
			players[i]->updateClientID(9999);
			players[i]->updatePlayerName(defaultName);
			playersConnected--;
			cout << "Player with ClientID " << clientID << " has left. Players left: " << playersConnected << endl;
			break;
		}
	}

	if (playersConnected < 4) {
		gameStarted = false;
		ball.stopGame();
	}
}

/* called when a client sends a message to the server */
void messageHandler(int clientID, string message){
	for (int i = 0; i < 4; i++) {
		if (players[i]->clientID == clientID) {
			if (message[0] == 'N') {

				players[i]->updatePlayerName(message.substr(1));
				break;
			}
			else {
				players[i]->updatePosition(message);
				break;
			}
		}
	}
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
		ball.moveBall();

		ostringstream os;
		os << ball.x_Pos << "|" << ball.y_Pos << "|" \
			<< player1.x_Pos << '|' << player1.y_Pos << '|' << player1.score << '|' << player1.playerName << '|' \
			<< player2.x_Pos << '|' << player2.y_Pos << '|' << player2.score << '|' << player2.playerName << '|' \
			<< player3.x_Pos << '|' << player3.y_Pos << '|' << player3.score << '|' << player3.playerName << '|' \
			<< player4.x_Pos << '|' << player4.y_Pos << '|' << player4.score << '|' << player4.playerName;

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
