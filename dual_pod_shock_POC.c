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
#define _transactionType 0xa0 /* Transaction Type is DATA (0xa0). Report Type is OUTPUT (0x02) */
#define _featuresSwitch 0xf3 /* 0xf0 Disables LED and Rumble Motors. 0xf3 Enables All of Them */
#define _powerRumbleRight 0x40 /* Power Rumble Right */
#define _powerRumbleLeft 0x00 /* Power Rumble Left */
#define _flashON 0x00 /* LED Flash On */
#define _flashOFF 0x00 /* LED Flash Off */
#define _volLeft 0x00 /* Volume Headset Speaker Left */
#define _volRight 0x00 /* Volume Headset Speaker Right */
#define _volMic	0x00 /* Volume Mic */
#define _volSpeaker 0x98 /* Volume Built-in Speaker / 0x4d == Uppercase M (Mute?) */
#define _R 0x10 /* Color Red */
#define _G 0x00 /* Color Green */
#define _B 0x00 /* Color Blue */


void printArray(unsigned char arr[], int size){
	int i;
	for (i=0; i < size; i++)
		printf("%02x ", arr[i]);

	printf("\n");
}

int main(int argc, char **argv){
    char *DS4 = "/dev/hidraw1";
    char *SBCFile = "SBC_Tracks/avemaria.sbc";
	int bufferSize = 334;
	size_t bytesRead = 0;
	char buf[bufferSize];
	char audioData[224];
	int fd, res;

	if (argc > 1)
		DS4 = argv[1];

	fd = open(DS4, O_RDWR|O_NONBLOCK);

	if(argc > 2)
		SBCFile = argv[2];

	if (fd < 0) {
		perror("Unable to open device");
		return 1;
	}

	/* P L A Y  T R A C K */
	FILE *file = NULL;
	file = fopen(SBCFile, "rb");
	if (file != NULL){
        		int lilEndianCounter = 0;
		while ((bytesRead = fread(audioData, 1, sizeof(audioData), file)) > 0){
			int indexBuffer = 81;
			int indexAudioData = 0;
			if(lilEndianCounter > 0xffff){
				lilEndianCounter = 0;
			}

			buf[0] = _protocolID; /* Protocol ID */
			buf[1] = _modeType; /* c0 Blueooth Mode, a0 USB Mode */
			buf[2] = _transactionType; /* Transaction Type is DATA (0xa0), Report Type is OUTPUT (0x02) */
			buf[3] = _featuresSwitch; /* 0xf0 disables the LEDs and rumble motors, 0xf3 enables them */
			buf[4] = 0x00; /* Unknown */
			buf[5] = 0x00; /* Unknown */
			buf[6] = _powerRumbleRight;/* Rumble Power Right */
			buf[7] = _powerRumbleLeft; /* Rumble Power Left */
			buf[8] = _R; /* Red */
			buf[9] = _G; /* Green */
			buf[10] = _B; /* Blue */
			buf[11] = _flashON; /* LED Flash On */
			buf[12] = _flashOFF; /* LED Flash Off */
			buf[13] = 0x00; buf[14] = 0x00; buf[15] = 0x00; buf[16] = 0x00; /* Start Empty Frames */
			buf[17] = 0x00; buf[18] = 0x00; buf[19] = 0x00; buf[20] = 0x00; /* End Empty Frames */
			buf[21] = _volLeft; /* Vol Left */
			buf[22] = _volRight; /* Vol Right */
			buf[23] = _volMic; /* Vol Mic */
			buf[24] = _volSpeaker; /* Vol Built-in Speaker / 0x4d == Uppercase M (Mute?) */
			buf[25] = 0x85; 											/* Unknown */
			buf[26] = 0x00; buf[27] = 0x00; buf[28] = 0x00; buf[29] = 0x00; buf[30] = 0x00; buf[31] = 0x00;	/* Start Empty Frames */
			buf[32] = 0x00; buf[33] = 0x00; buf[34] = 0x00;	buf[35] = 0x00; buf[36] = 0x00; buf[37] = 0x00;
			buf[38] = 0x00; buf[39] = 0x00; buf[40] = 0x00; buf[41] = 0x00; buf[42] = 0x00; buf[43] = 0x00;
			buf[44] = 0x00; buf[45] = 0x00; buf[46] = 0x00; buf[47] = 0x00; buf[48] = 0x00; buf[49] = 0x00;
			buf[50] = 0x00; buf[51] = 0x00; buf[52] = 0x00; buf[53] = 0x00; buf[54] = 0x00; buf[55] = 0x00;
			buf[56] = 0x00; buf[57] = 0x00; buf[58] = 0x00; buf[59] = 0x00; buf[60] = 0x00; buf[61] = 0x00;
			buf[62] = 0x00; buf[63] = 0x00; buf[64] = 0x00; buf[65] = 0x00; buf[66] = 0x00; buf[67] = 0x00;
			buf[68] = 0x00; buf[69] = 0x00; buf[70] = 0x00; buf[71] = 0x00; buf[72] = 0x00; buf[73] = 0x00;
			buf[74] = 0x00; buf[75] = 0x00; buf[76] = 0x00; buf[77] = 0x00; /* End Empty Frames */
			buf[78] = lilEndianCounter & 255; /* Audio frame counter (endian 1)*/
			buf[79] = (lilEndianCounter / 256) & 255; /* Audio frame counter (endian 2) */
			buf[80] = 0x02; /* 0x02 Speaker Mode On / 0x24 Headset Mode On*/

			// A U D I O  D A T A
			for(indexAudioData = 0; indexAudioData < sizeof(audioData); indexAudioData++){
				buf[indexBuffer++] = audioData[indexAudioData] & 255;
			}

    		buf[306] = 0x00; buf[307] = 0x00; buf[308] = 0x00; buf[309] = 0x00; buf[310] = 0x00; buf[311] = 0x00; /*Start Empty Frames */
			buf[312] = 0x00; buf[313] = 0x00; buf[314] = 0x00; buf[315] = 0x00; buf[316] = 0x00; buf[317] = 0x00;
			buf[318] = 0x00; buf[319] = 0x00; buf[320] = 0x00; buf[321] = 0x00; buf[322] = 0x00; buf[323] = 0x00;
			buf[324] = 0x00; buf[325] = 0x00; buf[326] = 0x00; buf[327] = 0x00; buf[328] = 0x00; buf[329] = 0x00; /* End Empty Frames */
			buf[330] = 0x00; buf[331] = 0x00; buf[332] = 0x00; buf[333] = 0x00; /* CRC-32 */
			res = write(fd, buf, bufferSize);

			if (res < 0) {
				printf("Error: %d\n", errno);
				perror("write");
			} else {
				lilEndianCounter += 2;
		 		printf("write() wrote %d bytes\n", res);
				printArray(buf, 334);
			}
		}
	}
}

