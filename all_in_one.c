/*
 *   all_in_one.c
 */

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pcf8591.h>
#include <wiringSerial.h>
#include <errno.h>
#include <string.h>

#define Address 0x48
#define BASE 64
#define A0 BASE+0
#define A1 BASE+1
#define A2 BASE+2
#define A3 BASE+3
#define D0 21
#define D1 22
#define D2 23
#define D3 26

#define MAXTIMINGS 85

#define DHTPIN 0

int dht11_dat[5] = {0,0,0,0,0};
int fd;

void append(char* s, char c)
{
        int len = strlen(s);
        s[len] = c;
}

void co2Reader(){
  int i = 0;
  int reading;
  serialPuts(fd,"Z\r\n");
  while(serialDataAvail(fd)){
    reading = serialGetchar(fd);
    if ((i > 2) && (i < 8)){
      printf("%d",reading-48);
    }
    fflush(stdout);
    ++i;
  }
  printf("00");
  fflush(stdout);
}

int read_dht11_dat()
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	float f; // fahrenheit
	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
	pinMode(DHTPIN, OUTPUT);
	digitalWrite(DHTPIN, LOW);
	delay(18);
	digitalWrite(DHTPIN, HIGH);
	delayMicroseconds(40); 
	pinMode(DHTPIN, INPUT);
	for ( i=0; i< MAXTIMINGS; i++) {
		counter = 0;
		while (digitalRead(DHTPIN) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 255) {
				break;
			}
		}
		laststate = digitalRead(DHTPIN);
		if (counter == 255) break;
		if ((i >= 4) && (i%2 == 0)) {
			// shove each bit into the storage bytes
			dht11_dat[j/8] <<= 1;
			if (counter > 50)
				dht11_dat[j/8] |= 1;
			j++;
		}
	}
	if ((j >= 40) && 
			(dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF)) ) {
		f = dht11_dat[2] * 9. / 5. + 32;
		printf("%d.%d,%.1f,", 
				dht11_dat[0], dht11_dat[1], f);
                return 1;
	}
        return 0;
}

int main (void)
{

  if ((fd = serialOpen ("/dev/ttyS0", 9600)) < 0)
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }
  serialPuts(fd,"M 4\r\n");
  serialPuts(fd,"K 2\r\n");
  serialFlush(fd);
 
	if (wiringPiSetup () == -1)
		exit (1) ;

        pcf8591Setup(BASE,Address);
        pinMode(D0,INPUT);
        pinMode(D1,INPUT);
        pinMode(D2,INPUT);
        pinMode(D3,INPUT);
	while (1) 
	{
		while (read_dht11_dat() == 0){}

		if (digitalRead(D0) == LOW){
                    printf("%04d,",analogRead(A0)*3300/255);
                }else{
                    printf("0000,");
                }
		if (digitalRead(D1) == LOW){
                    printf("%04d,",analogRead(A1)*3300/255);
                }else{
                    printf("0000,");
                }
		if (digitalRead(D2) == LOW){
                    printf("%04d,",analogRead(A2)*3300/255);
                }else{
                    printf("0000,");
                }
		if (digitalRead(D3) == LOW){
                    printf("%04d,",analogRead(A3)*3300/255);
                }else{
                    printf("0000,");
                }
                co2Reader();
		delay(300000); // wait 1sec to refresh
                printf("\n");
	}

	return 0 ;
}
