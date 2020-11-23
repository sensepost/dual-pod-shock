#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int _R = 0, _B = 0x0d, _G = 0, _volSpeaker = 85, _powerRumbleRight, _powerRumbleLeft, _bottomLEDs;

int main(int argc, char **argv){
  char *DS = "/dev/hidraw0";
  char bufWrite[66];
  char bufRead[16];
  int fd, res, pressedButton;

  if (argc > 1)
    DS = argv[1];

  fd = open(DS, O_RDWR|O_NONBLOCK);

  if (fd < 0){
    perror("Unable to open the Dual Sense");
    return 1;
  }

  memset(bufRead, 0x0, sizeof(bufRead));
  memset(bufWrite, 0x0, sizeof(bufWrite));

  for(;;){

    /* Get a report from the device */
    res = read(fd, bufRead, sizeof(bufRead));
  
    if (res > 0){
      for (int i = 0; i < res; i++){
        if(i == 8)
          pressedButton = bufRead[8] & 255;

        if(pressedButton == 0x06){ // Left
          _bottomLEDs = 0xc1;
        } else if(pressedButton == 0x04){ // Down
          _bottomLEDs = 0x00;
        } else if(pressedButton == 0x02){ // Right
          _bottomLEDs = 0xd0;
        } else  if(pressedButton == 0x00){ // Top
          _bottomLEDs = 0xff;
        } else  if(pressedButton == 0x18){ // Square
          _R = 0xff;
          _G = 0x14;
          _B = 0x93;
        } else  if(pressedButton == 0x28){ // Exe
          _R = 0x00;
          _G = 0x00;
          _B = 0xff;
        } else  if(pressedButton == 0x48){ // Circle
          _R = 0xff;
          _G = 0x00;
          _B = 0x00;
          _powerRumbleLeft = 0x85;
          _powerRumbleRight = 0x85;
        } else  if(pressedButton == 0x88){ // Triangle
          _R = 0x00;
          _G = 0xff;
          _B = 0x00;
          _powerRumbleLeft = 0x00;
          _powerRumbleRight = 0x00;
        }
      }    
    }

    /* Send a Report to the Device */
    bufWrite[0] = 0x15;
    bufWrite[1] = 0xff;
    bufWrite[2] = 0xf4;  // 0xff disable all LEDs - top LED, bottom LED, Mic LED (this just if bufWrite[63] == 0x02)// 0xf3 disable top LED// 0xf4 enable all LEDs - top LED, bottom LED, Mic LED (this just if buf[63] == 0x02)   
    bufWrite[3] = _powerRumbleRight;  // Right motor power
    bufWrite[4] = _powerRumbleLeft;  // Left motor power
    /* ... */
    bufWrite[9] = 0x00;  // Blinking Mic LED (if buf[2] == f7)
    /* ... */
    bufWrite[44] = _bottomLEDs; // LEDs from left to right -> 0xc1 0xa2 0xc4 0xc8 0xd0// 0x00 all OFF, 0xff all ON 
    bufWrite[45] = _R; // R
    bufWrite[46] = _G; // G
    bufWrite[47] = _B; // B
    /* ... */
    bufWrite[63] = 0x02; // Short Blink bottom led
    /* ... */
    bufWrite[65] = 0x02; // If both are set to 0xff it will turn OFF Motors and LEDs
    bufWrite[66] = 0x02; // else if both are set 0x02 it will turn ON Motors and LEDs

    res = write(fd, bufWrite, sizeof(bufWrite));
    
    if (res < 0){
      printf("Error: %d\n", errno);
      perror("write");
    } 
  }

  close(fd);
  return 0;
}