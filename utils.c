#include "utils.h"

char* base4code(long decimal, char* buffer) {
	int i;
	
	for (i = CODE_SIZE - 1; i >= 0; i--) {
		buffer[i] = '0' + (decimal & 1) ;
		decimal = decimal >> 1;
		buffer[i] = buffer[i] + 2 * (decimal & 1);
		decimal = decimal >> 1 ;
	}
    buffer[CODE_SIZE] = '\0';

    return buffer;
}

int base4(int decimal) {
    int digit, scale = 1, result = 0;

    while(decimal > 0) {
		digit = decimal % 4;
		result += digit * scale;
		decimal /= 4;
		scale *= 10;
    }
    return result;
}
