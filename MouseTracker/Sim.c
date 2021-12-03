	#include <stdlib.h>
	#include <stdio.h>
	#include <Windows.h>
	#include <time.h>
	#include <sys\timeb.h>
	#include <math.h>
	#include <time.h>

	typedef struct {
		int elapsedTime;
		int xlength;
		int ylength;
		float length;
		float angle;
		int startPos[2];
		int endPos[2];
		int status; //0 = stopped, 1 = moving
		char* statusMsg;
		double speed; //pixels per millisecond
		char* xdir;
		char* ydir;
	} vector;

	POINT p;

	int randomGenerator(int range);
	int cursorMove(float avgSpeed, float stdDevSpeed, float avgStopTime, int numStops, int goalX, int goalY);
	int addNoise();

	int main() {

		FILE* mousetrack_results = fopen("mtr.txt", "r");

		if (mousetrack_results == NULL) {
			return 0;
		}

		
		srand(time(0));

		int c;
		int numVals = 0;

		for (c = getc(mousetrack_results); c != EOF; c = getc(mousetrack_results)) {
			if (c == '\n') {
				numVals++;
			}
		}

		int* times = malloc(sizeof(int) * numVals);
		int* xvalues = malloc(sizeof(int) * numVals);
		int* yvalues = malloc(sizeof(int) * numVals);

		char line[16];
		int x;
		int y;
		int time;
		int co = 0;

		fclose(mousetrack_results);
		mousetrack_results = fopen("mtr.txt", "r");

		while (fgets(line, 16, mousetrack_results) != NULL) {
			sscanf(line, "%d,%d,%d", &time, &x, &y);
			printf("%d ", time);
			printf("%d ", x);
			printf("%d\n", y);
			times[co] = time;
			xvalues[co] = x;
			yvalues[co] = y;
			co++;
		}

		int size = co;

		for (int i = 0; i < size; i++) {
			printf("time = %d, x = %d, y = %d\n", times[i], xvalues[i], yvalues[i]);
		}

		int adjVecSize = size - 1;

		vector* adjacentVectors = malloc(sizeof(vector) * adjVecSize); //co is amount of values, co - 1 is amount of vectors
		float* speeds = malloc(sizeof(float) * adjVecSize);

		for (int i = 1; i < size; i++) {
			vector v;
			v.elapsedTime = times[i] - times[i - 1];
			v.startPos[0] = xvalues[i - 1];
			v.startPos[1] = yvalues[i - 1];
			v.endPos[0] = xvalues[i];
			v.endPos[1] = yvalues[i];
			v.xlength = v.endPos[0] - v.startPos[0];
			v.ylength = v.endPos[1] - v.startPos[1];
			v.length = sqrt((v.xlength * v.xlength) + (v.ylength * v.ylength));
			v.angle = ((180/3.1415) * atan2((v.startPos[1] - v.endPos[1]), (v.startPos[0] - v.endPos[0])));
			v.speed = v.length / v.elapsedTime;

			if (v.length == 0) {
				v.status = 0;
				v.statusMsg = "Stopped";
			}
			else {
				v.status = 1;
				v.statusMsg = "Moving";
			}

			if (v.xlength < 0) {
				v.xdir = "left";
			}
			else if (v.xlength > 0) {
				v.xdir = "right";
			}
			else {
				v.xdir = "null";
			}

			if (v.ylength < 0) {
				v.ydir = "up";
			}
			else if (v.ylength > 0) {
				v.ydir = "down";
			}
			else {
				v.ydir = "null";
			}

			adjacentVectors[i] = v;
			speeds[i] = v.speed;

		}

		//prints all vectors -- for testing
		for (int i = 0; i < size; i++) {
			vector v = adjacentVectors[i];
			printf("Vector num %d:\n", i);
			printf("time = %d, startPos = (%d,%d), endpos = (%d,%d)\n", v.elapsedTime, v.startPos[0], v.startPos[1], v.endPos[0], v.endPos[1]);
			printf("xlength = %d, ylength = %d, length = %.8f, angle = %.8f\n", v.xlength, v.ylength, v.length, v.angle);
			printf("status: %d, speed: %.8f, horizontal direction: %s, vertical direction: %s\n", v.status, v.speed, v.xdir, v.ydir);
		}


		float speedSum = 0;
		int speedCount = 0;
		double* cleanedSpeeds = malloc(sizeof(double) * adjVecSize);

		for (int i = 0; i < adjVecSize; i++) {
			printf("Speed: %.8f\n", speeds[i]);
			if (speeds[i] > 0.001) { //has rounding errors when comparing //don't include pauses in average speed
				printf("Added speed: %.8f\n", speeds[i]);
				speedSum += speeds[i];
				speedCount++;
				cleanedSpeeds[speedCount] = speeds[i];
			}
		}

		for (int i = 0; i < speedCount; i++) {
			printf("Sp: %.8f\n", cleanedSpeeds[i]);
		}

		float avgSpeed = speedSum / speedCount;
		printf("Average Speed: %f px/ms\n", avgSpeed);

		float speedSD = 0;
		for (int i = 0; i < speedCount; i++) {
			speedSD += ((cleanedSpeeds[i] - avgSpeed) * (cleanedSpeeds[i] - avgSpeed));
		}
		speedSD = sqrt(speedSD / speedCount);
		printf("StdDev Size for Speed: %f\n", speedSD);

		//calculate stop times by adding together adjacent stop times
		int stopStatusCount = 0; //number of times that user has a status of stopped
		int stopCount = 0; //number of individual stops
		float avgStopTime = 0; //may be multiple averages? calculated by taking stop time/adding consecutive stop times together
		float subAvgStopTime = 0;
		float longAvgStopTime = 0;
		int numSubAvgStops = 0; //number of stops considered shorter than average, will be used to simulate pauses
		int numNearAvgStops = 0; //number of stops considered roughly average in length, will be used to simulate stops
		int numLongStops = 0; //number of long stops, considered breaks, may or may not be necessary
		int numInactiveStops = 0; //number of very long stops, considered periods of inactivity and used for related calculations

		int* indivStopTimes = malloc(sizeof(int) * adjVecSize);

		//find stopping info
		int consecutiveStops = 0;
		int j = 0;
		int totalTime = 0;
		for (int i = 0; i < size; i++) {
			vector v = adjacentVectors[i];
			if (v.status == 0) {
				stopStatusCount++;
				//if isolated or first in streak:
				if (consecutiveStops == 0) {
					//if isolated
					if (adjacentVectors[i + 1].status == 1) {
						//add to invidStopTimes, increase j
						indivStopTimes[j] = v.elapsedTime;
						j++;
						//increase stopCount
						stopCount++;
					}
					//if first in streak
					else { //adjacentVectors[i + 1].status == 0
						//don't add to indivStopTimes yet, don't increase j yet
						//increase consecutiveStops
						consecutiveStops++;
						//increase total time
						totalTime += v.elapsedTime;
					}
				}
				//mid-streak or end of streak
				else { //if consecutiveStops is greater than 1
					//mid-streak
					if (adjacentVectors[i + 1].status == 0) {
						//don't add to indivStopTimes yet, don't increase j yet
						//increase consecutiveStops
						consecutiveStops++;
						//increase total time
						totalTime += v.elapsedTime;
					}
					//end of streak
					else { //if (adjacentVectors[i + 1].status == 1) {
						//reset consecutiveStops
						consecutiveStops = 0;
						//increase total time
						totalTime += v.elapsedTime;
						//add to indivStopTimes, increase j
						indivStopTimes[j] = totalTime;
						j++;
						//reset totalTime
						totalTime = 0;
						//increase stopCount
						stopCount++;
					}
				}
			}
		}

		//print times - for testing
		for (int i = 0; i < stopCount; i++) {
			printf("Time length at stop %d: %d\n", i, indivStopTimes[i]);
		}

		//calculate average stop time and standard deviation size
		int sum = 0;
		for (int i = 0; i < stopCount; i++) {
			sum += indivStopTimes[i];
		}
		avgStopTime = sum / stopCount;
		printf("Average Stop Time: %f\n", avgStopTime);
		float sd = 0;
		for (int i = 0; i < stopCount; i++) {
			sd += ((indivStopTimes[i] - avgStopTime) * (indivStopTimes[i] - avgStopTime));
		}
		sd = sqrt(sd / stopCount);
		printf("StdDev Size: %f\n", sd);
		subAvgStopTime = avgStopTime - sd;
		longAvgStopTime = avgStopTime + sd;
	

		fclose(mousetrack_results);
		GetCursorPos(&p);
		int xx = p.x;
		int yy = p.y;
		int sleep_time = 15;

		struct timeb start, stop;
		ftime(&start);

		//***sleep time determines speed
		//COUNT NUMBER OF TIMES IT PRINTS PER LOOP
		//random num seems to alter unrelated mouse movement somehow - be wary of changes
		
		int r = randomGenerator(100);
		printf("Generated %d\n", r);
		r = randomGenerator(100);
		printf("Generated %d\n", r);

		GetCursorPos(&p);
		printf("x, y = %d, %d\n", p.x, p.y);
		


		
		while (1) {
			//SetCursorPos(xx, yy);
			ftime(&stop);
			printf("%d\n", stop.time - start.time);
			//checks EVERY time the loop iterates
			cursorMove(avgSpeed, speedSD, avgSpeed, stopCount, 640, 360);
			if((stop.time - start.time) >= 5) {
				printf("%d\n", stop.time - start.time);
				exit(0);
			}
			Sleep(sleep_time);
			//xx++;
			//yy++;
		}
		
		

		//how to figure out the speed?
		/*
			TODO:
			0. Get a random number using C
			1. How can we use speed to determine how frequently the mouse should move?
				Do we need to measure loop time somehow?
			2. How can we implement stops every so often? How should the RNG work?
		*/

	}
	
	int randomGenerator(int range) {
		int r = (rand() % range + 1);
		return r;
	}

	//returns 1 if point has been reached ||||| SHOULD WORK BUT DOES NOT YET HAVE NOISE
	int cursorMove(float avgSpeed, float stdDevSpeed, float avgStopTime, int numStops, int goalX, int goalY) {
		printf("avgSpeed: %f, stdDevSpeed: %f, avgStopTime: %f, numStops: %d\n", avgSpeed, stdDevSpeed, avgStopTime, numStops);
		float sp = avgSpeed - stdDevSpeed;
		printf("sp: %f\n", sp);
		int c = randomGenerator(3);
		printf("c: %d\n", c);
		float r = avgSpeed;
		if (c == 1) {
			r = avgSpeed - sp;
		}
		else if (c == 3) {
			r = avgSpeed + sp;
		}
		printf("r: %f\n", r);
		int range = (int)(r * 100);
		printf("Range: %d\n", range);
		//int moves = randomGenerator(range);
		int moves = 100;
		printf("Moves: %d\n", moves);

		//whether x and y should be moved
		//int moveX = 1;
		//int moveY = 1;

		for (int i = 0; i < moves; i++) {
			int shouldPause = randomGenerator(10);
			int noX = randomGenerator(20);
			int noY = randomGenerator(20);
			int moreX = randomGenerator(5);
			int moreY = randomGenerator(5);
			GetCursorPos(&p);
			int x = p.x;
			int y = p.y;
			if (p.x == goalX && p.y == goalY) {
				return 1;
			}
			if (p.x > goalX) {
				if (noX > 3) {
					x--;
				}
				if (moreX <= 2) {
					int more = randomGenerator(5);
					for (int j = 0; j < more; j++) {
						x--;
					}
				}
			}
			else if (p.x < goalX) {
				if (noX > 3) {
					x++;
				}
				if (moreX <= 2) {
					int more = randomGenerator(10);
					for (int j = 0; j < more; j++) {
						x++;
					}
				}
			}
			else {
				if(p.y != goalY && p.x != goalX - 15 && p.x != goalX + 15){
					int haveNoise = randomGenerator(3);
					if (haveNoise == 3) {
						int Noise = randomGenerator(5);
						int NoiseDir = randomGenerator(2);
						//for (int j = 0; j < Noise; j++) {
							if (NoiseDir == 1) {
								x--;
							}
							else {
								x++;
							}
						//}
					}
				}
			}
			if (p.y > goalY) {
				if (noY > 3) {
					y--;
				}
				if (moreY <= 2) {
					int more = randomGenerator(10);
					for (int j = 0; j < more; j++) {
						y--;
					}
				}
			}
			else if (p.y < goalY) {
				if (noY > 3) {
					y++;
				}
				if (moreY <= 2) {
					int more = randomGenerator(10);
					for (int j = 0; j < more; j++) {
						y++;
					}
				}
			}
			else {
				if (p.x != goalX && p.y != goalY - 15 && p.y != goalY + 15) {
					int haveNoise = randomGenerator(3);
					if (haveNoise == 3) {
						int Noise = randomGenerator(5);
						int NoiseDir = randomGenerator(2);
						//for (int j = 0; j < Noise; j++) {
							if (NoiseDir == 1) {
								y--;
							}
							else {
								y++;
							}
						//}
					}
				}
			}
			if (shouldPause > 7) {
				int pause = randomGenerator(50);
				Sleep(pause);
			}
			SetCursorPos(x, y);
		}
		return 0;
	}


	int addNoise() {
	int haveNoise = randomGenerator(3);
	if (haveNoise == 3) {
		int Noise = randomGenerator(5);
		int NoiseDir = randomGenerator(2);
		for (int j = 0; j < Noise; j++) {
			if (NoiseDir == 1) {
				return -1;
			}
			else {
				return 1;
			}
		}
	}
	else {
		return 0;
	}
}
