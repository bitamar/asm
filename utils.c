long to_base4(long decimal) {
    int digit;
    long scale, result = 0L;

    scale = 1;

    while(decimal > 0) {
		digit = decimal % 4;
		result += digit * scale;
		decimal /= 4;
		scale *= 10;
    }
    return result;
}
