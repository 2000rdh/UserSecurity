#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <sys\timeb.h>

int main() {

	FILE *mousetrack_results = fopen("mtr.txt", "w"); //a to append
	fclose(mousetrack_results);

	POINT p;

	//time_t start, stop;
	struct timeb start, stop;
	int time;
	int i;

	ftime(&start);

	while(1) {
		mousetrack_results = fopen("mtr.txt", "a");
		int clickStatus = 0; //0 = not clicked, 1 = left click, 2 = middle click, 3 = right click
		//printf("running\n");
		if (GetCursorPos(&p)) {
			ftime(&stop);
		/*	time = (int)(1000.0 * (stop.time - start.time)
				+ (stop.millitm - start.millitm));*/
			/*printf("Time elapsed: %ld\n", time);
			printf("P.x = %d ", p.x);
			printf("P.y = %d\n", p.y);*/
			if ((GetKeyState(MK_LBUTTON))) {
				clickStatus = 1;
			}
			else if ((GetKeyState(MK_MBUTTON))) {
				clickStatus = 2;
			}
			else if ((GetKeyState(MK_RBUTTON))) {
				clickStatus = 3;
			}
			fprintf(mousetrack_results, "%d,%d,%d,%d\n", (int)(1000.0 * (stop.time - start.time)
				+ (stop.millitm - start.millitm)), p.x, p.y, clickStatus);
			/*fprintf(mousetrack_results, "%d,", time);
			fprintf(mousetrack_results, "%d,", p.x);
			fprintf(mousetrack_results, "%d\n", p.y);*/
			/*fprintf(mousetrack_results, "P.x = %d ", p.x);
			fprintf(mousetrack_results, "P.y = %d\n", p.y);*/
		}
		fclose(mousetrack_results);
		Sleep(250);
	}

	fclose(mousetrack_results);
	return 0;
}
//SHOULD BE MEASURING CLICKS
//points a and b determined by clicks AND stops? and stop under a certain threshold are considered pauses?