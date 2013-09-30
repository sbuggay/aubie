#include <stdio.h>
#include <stdint.h>

uint8_t calc_checksum(uint8_t *p, int size) {
	int i = 0;
	uint8_t buffer = 0;
	for (i; i < size; i++) {
		buffer += p[i];
		printf("%u\n", buffer);
	}
	return buffer;
}

int main(int argc, char *argv[]) {
	uint8_t list[4];
	list[0] = 127;
	list[1] = 170;
	list[2] = 100;
	list[3] = 40;
	calc_checksum(list, 4);
	return 0;
}