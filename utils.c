
char * to_base4(long decimal,int num_of_digit,char * str_to_return) {
	int i;
	
	for (i=num_of_digit-1;i>=0;i--) {
		str_to_return[i]='0'+ (decimal & 1) ;
		decimal=decimal>>1 ;
		str_to_return[i]=str_to_return[i]+ 2* (decimal & 1) ;
		decimal=decimal>>1 ;
	}
    str_to_return[num_of_digit]='\0';
    return str_to_return;
}
