#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <chrono>
#include <random>
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

	double lastCalculatedLatency = 0.9999;
	chrono::milliseconds lastCalculatedTimeStamp;

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
			if (message == "R") {
				x_Speed = 4;
				if ((x_Pos + width) + x_Speed > 700) {
					x_Speed = 0;
					x_Pos = 700 - width;
				}
				else {
					x_Pos += x_Speed;
				}
			}
			else if (message == "L") {
				x_Speed = -4;
				if (x_Pos + x_Speed < 0) {
					x_Speed = 0;
					x_Pos = 0;
				}
				else {
					x_Pos += x_Speed;
				}
			}
			else if (message == "S") {
				x_Speed = 0;
			}
		}
		else if (orientation == "VERTICAL") {
			if (message == "U") {
				y_Speed = -4;
				if (y_Pos + y_Speed < 0) {
					y_Speed = 0;
					y_Pos = 0;
				}
				else {
					y_Pos += y_Speed;
				}
			}
			else if (message == "D") {
				y_Speed = 4;
				if ((y_Pos + height) + y_Speed > 700) {
					y_Speed = 0;
					y_Pos = 700 - height;
				}
				else {
					y_Pos += y_Speed;
				}
			}
			else if (message == "S") {
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
			x_Speed = 0;
			y_Speed = -3;
			givePoint();
		}
		else if (topX < 0) {			// Player 3
			x_Pos = 5;
			x_Speed = 0;
			y_Speed = 3;
			givePoint();
		}
		else if (bottomY > 700) {		// Player 1
			y_Pos = 695;
			y_Speed = 0;
			x_Speed = 3;
			givePoint();
		}
		else if (topY < 0) {			// Player 2
			y_Pos = 5;
			y_Speed = 0;
			x_Speed = -3;
			givePoint();
		}

		for (int i = 0; i < 4; i++) {
			if (topY < (players[i]->y_Pos + players[i]->height) && bottomY > players[i]->y_Pos && topX < (players[i]->x_Pos + players[i]->width) && bottomX > players[i]->x_Pos) {
				if (players[i]->orientation == "HORIZONTAL") {
					x_Speed += int(players[i]->x_Speed / 2);
					y_Speed = y_Speed * -1;
				}
				if (players[i]->orientation == "VERTICAL") {
					x_Speed = x_Speed * -1;
					y_Speed += int(players[i]->y_Speed / 2);
				}
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
			y_Speed = 3;
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

pongBall ball{ 350, 350, 6, 0, 0, players };
bool gameStarted = false;

// Latency Testing Booleans ---------------------------------------------------------------------------------------------------------
// ONLY ONE SHOULD BE TRUE AT ANY POINT.
bool useFixedLatency = false;
float latencyScalar = 6.0;			// Change this value to increase the latency in sending a message back to clients

bool useRandomLatency = false;
default_random_engine generator;
uniform_int_distribution<int> dist(35, 340);

bool useIncrementalLatency = false;
int latency = 20;
int latencyAcc = 10;
int latencyMax = 400;

bool useSynchronizedLatency = false;

// END DEFINES HERE.

/* called when a client connects */
void openHandler(int clientID) {
	if (playersConnected == 4) {
		server.wsClose(clientID);
	}

	for (int i = 0; i < 4; i++) {
		if (players[i]->clientID == 9999) {
			cout << "Player " << i + 1 << " has joined. Client ID: " << clientID << endl;
			players[i]->updateClientID(clientID);
			playersConnected++;
			

			ostringstream os;
			os << "PN" << '|' <<i+1;
			string serverMessage = os.str();
			server.wsSend(players[i]->clientID, serverMessage);

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
void closeHandler(int clientID) {
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
void messageHandler(int clientID, string message) {
	for (int i = 0; i < 4; i++) {
		if (players[i]->clientID == clientID) {
			if (message[0] == 'N') {
				players[i]->updatePlayerName(message.substr(1));

				ostringstream os;
				os << ball.x_Pos << "|" << ball.y_Pos << "|" \
					<< player1.x_Pos << '|' << player1.y_Pos << '|' << player1.score << '|' << player1.playerName << '|' \
					<< player2.x_Pos << '|' << player2.y_Pos << '|' << player2.score << '|' << player2.playerName << '|' \
					<< player3.x_Pos << '|' << player3.y_Pos << '|' << player3.score << '|' << player3.playerName << '|' \
					<< player4.x_Pos << '|' << player4.y_Pos << '|' << player4.score << '|' << player4.playerName;
				string serverMessage = os.str();

				vector<int> clientIDs = server.getClientIDs();
				for (int i = 0; i < clientIDs.size(); i++) {
					server.wsSend(clientIDs[i], serverMessage);
				}

				break;
			}
			else {
				if (gameStarted) {
					string moveDir = message.substr(0, 1);

					long long clientTS = stoll(message.substr(2), 0, 0);
					chrono::milliseconds cts{ clientTS };
					players[i]->lastCalculatedTimeStamp = cts;

					chrono::time_point<chrono::system_clock> serverTime = chrono::system_clock::now();
					players[i]->lastCalculatedLatency = chrono::duration<double, std::milli>(serverTime.time_since_epoch() - cts).count();

					players[i]->updatePosition(moveDir);
					break;
				}
			}
		}
	}
}

/* called once per select() loop */
void periodicHandler() {
	ball.moveBall();

	static chrono::milliseconds frame{ 17 };
	static chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

	chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
	chrono::duration<double, std::milli> time_span = t2 - t1;
	ostringstream os;

	static chrono::time_point<chrono::system_clock> t3 = chrono::system_clock::now();
	
	if (!useFixedLatency && !useRandomLatency && !useIncrementalLatency && !useSynchronizedLatency) {
		if (time_span.count() >= frame.count() && gameStarted) {
			

			os << ball.x_Pos << "|" << ball.y_Pos << "|" \
				<< player1.x_Pos << '|' << player1.y_Pos << '|' << player1.score << '|' << player1.playerName << '|' \
				<< player2.x_Pos << '|' << player2.y_Pos << '|' << player2.score << '|' << player2.playerName << '|' \
				<< player3.x_Pos << '|' << player3.y_Pos << '|' << player3.score << '|' << player3.playerName << '|' \
				<< player4.x_Pos << '|' << player4.y_Pos << '|' << player4.score << '|' << player4.playerName << '|' \
				<< chrono::duration_cast<chrono::milliseconds>(t3.time_since_epoch()).count();
			string serverMessage = os.str();

			vector<int> clientIDs = server.getClientIDs();
			for (int i = 0; i < clientIDs.size(); i++) {
				server.wsSend(clientIDs[i], serverMessage);
			}

			//for (int i = 0; i < 4; i++) {
			//	cout << "player " << i + 1 << " latency: " << players[i]->lastcalculatedlatency << "ms" << endl;
			//}

			t1 = chrono::high_resolution_clock::now();
			t3 = chrono::system_clock::now();
		}
	}
	else if (useFixedLatency) {
		if (time_span.count() >= frame.count() * latencyScalar && gameStarted) {
			ostringstream os;
			os << ball.x_Pos << "|" << ball.y_Pos << "|" \
				<< player1.x_Pos << '|' << player1.y_Pos << '|' << player1.score << '|' << player1.playerName << '|' \
				<< player2.x_Pos << '|' << player2.y_Pos << '|' << player2.score << '|' << player2.playerName << '|' \
				<< player3.x_Pos << '|' << player3.y_Pos << '|' << player3.score << '|' << player3.playerName << '|' \
				<< player4.x_Pos << '|' << player4.y_Pos << '|' << player4.score << '|' << player4.playerName << '|' \
				<< chrono::duration_cast<chrono::milliseconds>(t3.time_since_epoch()).count();
			string serverMessage = os.str();

			vector<int> clientIDs = server.getClientIDs();
			for (int i = 0; i < clientIDs.size(); i++)
				server.wsSend(clientIDs[i], serverMessage);

			t1 = chrono::high_resolution_clock::now();
			t3 = chrono::system_clock::now();
		}
	}
	else if (useRandomLatency) {
		chrono::milliseconds randomFrame{ dist(generator) };
		if (time_span.count() >= randomFrame.count() && gameStarted) {
			ostringstream os;
			os << ball.x_Pos << "|" << ball.y_Pos << "|" \
				<< player1.x_Pos << '|' << player1.y_Pos << '|' << player1.score << '|' << player1.playerName << '|' \
				<< player2.x_Pos << '|' << player2.y_Pos << '|' << player2.score << '|' << player2.playerName << '|' \
				<< player3.x_Pos << '|' << player3.y_Pos << '|' << player3.score << '|' << player3.playerName << '|' \
				<< player4.x_Pos << '|' << player4.y_Pos << '|' << player4.score << '|' << player4.playerName << '|' \
				<< chrono::duration_cast<chrono::milliseconds>(t3.time_since_epoch()).count();
			string serverMessage = os.str();

			vector<int> clientIDs = server.getClientIDs();
			for (int i = 0; i < clientIDs.size(); i++)
				server.wsSend(clientIDs[i], serverMessage);

			t1 = chrono::high_resolution_clock::now();
			t3 = chrono::system_clock::now();
		}
	}

	else if (useIncrementalLatency) {
		chrono::milliseconds incLatency{ latency };
		if (time_span.count() >= incLatency.count() && gameStarted) {
			ostringstream os;
			os << ball.x_Pos << "|" << ball.y_Pos << "|" \
				<< player1.x_Pos << '|' << player1.y_Pos << '|' << player1.score << '|' << player1.playerName << '|' \
				<< player2.x_Pos << '|' << player2.y_Pos << '|' << player2.score << '|' << player2.playerName << '|' \
				<< player3.x_Pos << '|' << player3.y_Pos << '|' << player3.score << '|' << player3.playerName << '|' \
				<< player4.x_Pos << '|' << player4.y_Pos << '|' << player4.score << '|' << player4.playerName << '|' \
				<< chrono::duration_cast<chrono::milliseconds>(t3.time_since_epoch()).count();
			string serverMessage = os.str();

			vector<int> clientIDs = server.getClientIDs();
			for (int i = 0; i < clientIDs.size(); i++)
				server.wsSend(clientIDs[i], serverMessage);

			t1 = chrono::high_resolution_clock::now();
			t3 = chrono::system_clock::now();

			if (latency < latencyMax) {
				latency += latencyAcc;
			}
		}
	}
	else if (useSynchronizedLatency) {
		if (time_span.count() >= frame.count() && gameStarted) {
			
			chrono::milliseconds lowest{ 999999999999999999 };
			for (int i = 0; i < 4; i++) {
				if (players[i]->lastCalculatedTimeStamp<lowest) {
					lowest = players[i]->lastCalculatedTimeStamp;
				}
			}
			os << ball.x_Pos << "|" << ball.y_Pos << "|" \
				<< player1.x_Pos << '|' << player1.y_Pos << '|' << player1.score << '|' << player1.playerName << '|' \
				<< player2.x_Pos << '|' << player2.y_Pos << '|' << player2.score << '|' << player2.playerName << '|' \
				<< player3.x_Pos << '|' << player3.y_Pos << '|' << player3.score << '|' << player3.playerName << '|' \
				<< player4.x_Pos << '|' << player4.y_Pos << '|' << player4.score << '|' << player4.playerName << '|' \
				<< chrono::duration_cast<chrono::milliseconds>(t3.time_since_epoch()).count();
			string serverMessage = os.str();

			vector<int> clientIDs = server.getClientIDs();
			for (int i = 0; i < clientIDs.size(); i++) {
				server.wsSend(clientIDs[i], serverMessage);
			}

			//for (int i = 0; i < 4; i++) {
			//	cout << "Player " << i + 1 << " latency: " << players[i]->lastCalculatedLatency << "ms" << endl;
			//}

			t1 = chrono::high_resolution_clock::now();
			t3 = chrono::system_clock::now();
		}
	}
}

int main(int argc, char *argv[]) {
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
