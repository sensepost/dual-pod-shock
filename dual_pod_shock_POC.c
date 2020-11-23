#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/* S E T T I N G S  D U A L - P O D - S H O C K  4 */
#define _protocolID 0x15 /* Protocol ID */
#define _modeType 0xc0 /* c0 Bluetooth Mode / a0 USB Mode */
#define _transactionType 0xa2 /* Transaction Type is DATA (0xa0). Report Type is OUTPUT (0x02) */
#define _featuresSwitch 0xf3 /* 0xf0 Disables LED and Rumble Motors. 0xf3 Enables All of Them */
#define _volLeft 0x43 /* Volume Headset Speaker Left */
#define _volRight 0x43 /* Volume Headset Speaker Right */
#define _volMic  0x4f /* Volume Mic */
#define _flashON 0x00 /* LED Flash On */
#define _flashOFF 0x00 /* LED Flash Off */


int _R = 0, _B = 0x0d, _G = 0, _volSpeaker = 85, _powerRumbleRight, _powerRumbleLeft;

int main(int argc, char **argv){
  char *DS4 = "/dev/hidraw0";
  char *SBC_Tracks[6] = { "SBC_Tracks/Manu Chao - Me gustas tu.sbc", "SBC_Tracks/Mario - Let Me Love You.sbc", "SBC_Tracks/Haddaway - What is love.sbc", "SBC_Tracks/Bella Ciao - Modena City Ramblers.sbc", "SBC_Tracks/Schubert - Ave Maria.sbc", "SBC_Tracks/Roy Orbison - Oh, my love, my darling.sbc" };
  char bufWrite[334];
  char bufRead[16];
  char audioData[224];
  size_t bytesRead = 0;
  int fd, res, pressedButton;
  
  if (argc > 1)
    DS4 = argv[1];

  fd = open(DS4, O_RDWR|O_NONBLOCK);

  if (fd < 0){
    perror("Unable to open the Dual Shock");
    return 1;
  }

  memset(bufRead, 0x0, sizeof(bufRead));
  memset(bufWrite, 0x0, sizeof(bufWrite));

  for(;;){

    size_t tn = 0;
    for(tn = 0; tn < sizeof(SBC_Tracks) / sizeof(SBC_Tracks[0]); tn++){
         
      /* P L A Y  T R A C K */
      FILE *file = NULL;
      file = fopen(SBC_Tracks[tn], "rb");
      if (file != NULL){
        int lilEndianCounter = 0;
        while ((bytesRead = fread(audioData, 1, sizeof(audioData), file)) > 0){
          int indexBuffer = 81;
          int indexAudioData = 0;
          if(lilEndianCounter > 0xffff){
            lilEndianCounter = 0;
          }
          
          bufWrite[0] = _protocolID; /* Protocol ID */
          bufWrite[1] = _modeType; /* c0 Blueooth Mode, a0 USB Mode */
          bufWrite[2] = _transactionType; /* Transaction Type is DATA (0xa0), Report Type is OUTPUT (0x02) */
          bufWrite[3] = _featuresSwitch; /* 0xf0 disables the LEDs and rumble motors, 0xf3 enables them */
          bufWrite[4] = 0x44; /* Unknown */
          bufWrite[5] = 0x00; /* Unknown */
          bufWrite[6] = _powerRumbleRight;/* Rumble Power Right */
          bufWrite[7] = _powerRumbleLeft; /* Rumble Power Left */
          bufWrite[8] = _R; /* Red */
          bufWrite[9] = _G; /* Green*/
          bufWrite[10] = _B; /* Blue */
          bufWrite[11] = _flashON; /* LED Flash On */
          bufWrite[12] = _flashOFF; /* LED Flash Off */
          /* ... */
          bufWrite[20] = _volLeft; /* Vol Left */
          bufWrite[21] = _volRight; /* Vol Right */ 
          bufWrite[22] = 0x00; /* Unknown */
          bufWrite[23] = _volMic; /* Vol Mic */
          bufWrite[24] = _volSpeaker; /* Vol Built-in Speaker */
          bufWrite[25] = 0x40; /* Unknown */
          /* ... */
          bufWrite[78] = lilEndianCounter & 255; /* Audio frame counter (endian 1)*/
          bufWrite[79] = (lilEndianCounter / 256) & 255; /* Audio frame counter (endian 2) */
          bufWrite[80] = 0x02; /* 0x02 Speaker Mode On / 0x24 Headset Mode On*/

          // A U D I O  D A T A
          for(indexAudioData = 0; indexAudioData < sizeof(audioData); indexAudioData++){
            bufWrite[indexBuffer++] = audioData[indexAudioData] & 255;
          }

          /* ... */
          bufWrite[330] = 0x00; bufWrite[331] = 0x00; bufWrite[332] = 0x00; bufWrite[333] = 0x00; /* CRC-32 */
          
          /* G E T  P R E S S E D  K E Y */
          /* Get a report from the device to identify the pressed key */
          res = read(fd, bufRead, sizeof(bufRead));
          if (res > 0) {
            for (int i = 0; i < res; i++){
              if(i == 7)
                pressedButton = bufRead[7] & 255;

              if(pressedButton == 0x18){ /* Square - It will decrease the volume */
                _R = 0xff;
                _G = 0x14;
                _B = 0x93;
                _volSpeaker = 40; /* Minumum Volume Audible */
              } else if(pressedButton == 0x28){ /* Exe - It will increase the volume */
                _R = 0x00;
                _G = 0x00;
                _B = 0xff;
                _volSpeaker = 85;
              } else if(pressedButton == 0x48){ /* Circle - It will turn on the shock motors */
                _R = 0xff;
                _G = 0x00;
                _B = 0x00;
                _powerRumbleLeft = 0x85; // Switch ON left motor
                _powerRumbleRight = 0x85; // Switch ON right motor
              } else  if(pressedButton == 0x88){ /* Triangle - It will switch off the shock motors */
                _R = 0x00;
                _G = 0xff;
                _B = 0x00;
                _powerRumbleLeft = 0x00; // Switch OFF left motor
                _powerRumbleRight = 0x00; // Switch OFF right motor
              }
            }    
          }

          res = write(fd, bufWrite, sizeof(bufWrite));

          if (res < 0){
            printf("Error: %d\n", errno);
            perror("write");
          } else {
            lilEndianCounter += 2;
          }
        }
      }
    }
  }

  close(fd);
  return 0;
}