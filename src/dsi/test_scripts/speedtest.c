#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
	int vValue, i;
        float vRet;
	clock_t vTime;
	vTime = clock();
	for (i = 0; i < 150000; ++i) {
		vValue = 1145677;
		vRet = (vValue & 2047) / 8.0f;
		vRet = ((vValue >> 11) & 2047) / 8.0f;
			vRet = ((vValue >> 22) & 1023) / 4.0f;
        }
	vTime = clock() - vTime;
	printf("Finished Test, %fs.\n", (float) vTime / CLOCKS_PER_SEC);
	return EXIT_SUCCESS;
}
