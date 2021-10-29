#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <math.h>
#

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
} vector;

//typedef struct {
//    int x;
//    int y;
//    //int timestamp;
//} mousePos;
//
//mousePos getMousePos();

int main() {

	FILE* mousetrack_results = fopen("mtr.txt", "r");

	//may need to handle a different way

	if (mousetrack_results == NULL) {
		return 0;
	}

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
		v.angle = atan2((v.startPos[1] - v.endPos[1]), (v.startPos[0] - v.endPos[0]));

		if (v.length == 0) {
			v.status = 0;
			v.statusMsg = "Stopped";
		}
		else {
			v.status = 1;
			v.statusMsg = "Moving";
		}

		adjacentVectors[i] = v;

	}

	//prints all vectors -- for testing
	for (int i = 0; i < size; i++) {
		vector v = adjacentVectors[i];
		printf("Vector num %d:\n", i);
		printf("time = %d, startPos = (%d,%d), endpos = (%d,%d)\n", v.elapsedTime, v.startPos[0], v.startPos[1], v.endPos[0], v.endPos[1]);
		printf("xlength = %d, ylength = %d, length = %.8f, angle = %.8f\n", v.xlength, v.ylength, v.length, v.angle);
		printf("status: %d\n", v.status);
	}

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
	//^^^will probably only need these

	//point a to point b is a number of clicks or a stop at a certain length
	//every time a boolean value changes add to counter





	

			/*for (c = getc(mousetrack_results); c != EOF; c = getc(mousetrack_results)) {
				printf("loop\n");
				fscanf(mousetrack_results, "%d", x);
				printf("%d ", x);
			}*/

		//printf("%d\n", numVals);

		//while	

		//simulate movement from point a to point b
		/*int x = 100;
		int y = 100;
		SetCursorPos(x, y);*/



		/*
		int currx = 100;
		while (currx != 200) {
			SetCursorPos(++x, y);
		}
		*/
		//every time it loops through it checks where it needs to get to and whether its lesser or greater and moves accordingly
		//(moves to right if x < objective coordinate)
		//SetCursorPos(100, 100);

		fclose(mousetrack_results);

	}