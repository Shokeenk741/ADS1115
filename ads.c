// ads1115b.c read pot on ANC1
// output value in volts exit program.
// uses one-shot mode
// pull up resistors in module caused problems
// used level translator - operated ADS1115 at 5V
// by Lewis Loflin lewis@bvu.net
// www.bristolwatch.com
// http://www.bristolwatch.com/rpi/ads1115.html

#include <stdio.h>
#include <sys/types.h> // open
#include <sys/stat.h>  // open
#include <fcntl.h>     // open
#include <unistd.h>    // read/write usleep
#include <stdlib.h>    // exit
#include <inttypes.h>  // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions
#include <sys/ioctl.h>
#include <inttypes.h>  // uint8_t, etc

//ADS1115 possible addresses
#define HEX48 0b1001000  //Hex 48
//Define 1-shot read or continuous
#define ONESHOT_ON  0b10000001
#define ONESHOT_OFF 0

//Define Analog Inputs
#define AI0 0b01000000
#define AI1 0b01010000
#define AI2 0b01100000
#define AI3 0b01110000

//Define gain setting
#define GAIN1 0b00000000  //Full Scale Voltage is: 6.144 + or -
#define GAIN2 0b00000010  //FS 4.096
#define GAIN3 0b00000100  //FS 2.048

int fd;
int ads_address = 0x48;
const float VPS = 6.144/32768.0; //volts per step
const float VPS2 = 4.096/32768.0; //volts per step

int16_t ads_read( int address, int shot1, int ain, int gain ){

  // int fd;
   int16_t val;
   uint8_t writeBuf[3], readBuf[2];

   // connect to ads1115 as i2c slave
   if (ioctl(fd, I2C_SLAVE, address) < 0) {
      printf("Error: Couldn't find device on address!\n");
      exit (1);
   }

   // set config register and start conversion

   writeBuf[0] = 1;    // config register is 1
   writeBuf[1] = shot1 + ain + gain;
   writeBuf[2] = 0b10000101; // bits 7-0  0x85

   // begin conversion
   if (write(fd, writeBuf, 3) != 3) {
       perror("Can't write to register 1");
       exit(-1);
   }

   // wait for conversion complete
   // checking bit 15

   do{
      if (read(fd, writeBuf, 2) != 2) {
        perror("Read conversion failure");
        exit(-1);
      }
   }while ((writeBuf[0] & 0x80) == 0);

   // read conversion register
   // write register pointer first

   readBuf[0] = 0;   // conversion register is 0

   if (write(fd, readBuf, 1) != 1) {
      perror("Write register select failure");
      exit(-1);
   }

   // read 2 bytes
   if (read(fd, readBuf, 2) != 2) {
      perror("Read conversion failure");
      exit(-1);
   }

   // convert display results
   val = readBuf[0] << 8 | readBuf[1];

   if (val < 0)   val = 0;

//   close(fd);

   return val;
}

int main() {


  int16_t analog_in;
  // open device on /dev/i2c-1 
  // the default on Raspberry Pi B
  if ((fd = open("/dev/i2c-1", O_RDWR)) < 0) {
    printf("Error: Couldn't open device! %d\n", fd);
    exit (1);
  }



  while(1){

        analog_in = ads_read( HEX48, ONESHOT_ON, AI0, GAIN1 );
        printf("ADS AI0 voltage is: %f\n", (analog_in * VPS));
        analog_in = ads_read( HEX48, ONESHOT_ON, AI1, GAIN1 );
        printf("ADS AI1 voltage is: %f\n", (analog_in * VPS));
        analog_in = ads_read( HEX48, ONESHOT_ON, AI2, GAIN1 );
        printf("ADS AI2 voltage is: %f\n", (analog_in * VPS));
	analog_in = ads_read( HEX48, ONESHOT_ON, AI3, GAIN1 );
        printf("ADS AI3 voltage is: %f\n", (analog_in * VPS));

        printf("\n\n\n\n");
        usleep(400000);

   }
  return 0;

}
