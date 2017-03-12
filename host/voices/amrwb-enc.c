/* ------------------------------------------------------------------
 * Copyright (C) 2009 Martin Storsjo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <enc_if.h>
#include <unistd.h>
#include <stdlib.h>

#define WAV_HEAD sizeof(struct wave_pcm_hdr)
typedef int SR_DWORD;
typedef short int SR_WORD ;

struct wave_pcm_hdr{ //音频头部格式
         char            riff[4];                        // = "RIFF"
         SR_DWORD        size_8;                         // = FileSize - 8
         char            wave[4];                        // = "WAVE"
         char            fmt[4];                         // = "fmt "
         SR_DWORD        dwFmtSize;                      // = 下一个结构体的大小 : 16
 
         SR_WORD         format_tag;              // = PCM : 1
         SR_WORD         channels;                       // = 通道数 : 1
         SR_DWORD        samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
         SR_DWORD        avg_bytes_per_sec;      // = 每秒字节数 : dwSamplesPerSec * wBitsPerSample / 8
         SR_WORD         block_align;            // = 每采样点字节数 : wBitsPerSample / 8
         SR_WORD         bits_per_sample;         // = 量化比特数: 8 | 16
 
         char            data[4];                        // = "data";
         SR_DWORD        data_size;                // = 纯数据长度 : FileSize - 44 
};

struct wave_pcm_hdr pcmwavhdr;

int WavAmr16k(const void *data,void *outdata,int *size){
	int mode = 8;
	int dtx = 0;
	struct wave_pcm_hdr *hdr=NULL;
	FILE *out=NULL;
	int inputSize,outsize=0;
	uint8_t* inputBuf;
	int format, sampleRate, channels, bitsPerSample,AllSize=0,readSize=0;
	hdr = (struct wave_pcm_hdr *)data;

	if(hdr->samples_per_sec!=16000){
		return -1;
	}
	if(hdr->channels!=1){
		return -1;
	}
	channels = hdr->channels;
	inputSize = hdr->channels*2*320;
	inputBuf = (uint8_t*) malloc(inputSize);

	AllSize = hdr->data_size;
	void  *amr=NULL;
	amr = E_IF_init();
	out = fopen("./cache16k.tmp", "wb");
	fwrite("#!AMR-WB\n", 1, 9, out);
	memcpy(outdata+outsize,"#!AMR-WB\n",9);
	printf("start enc \n");
	readSize+=sizeof(struct wave_pcm_hdr);
	outsize+=9;
	int eixt=1;
	while (eixt) {
		int read, i, n;
		short buf[320];
		uint8_t outbuf[500];
		if(readSize+inputSize>AllSize){
			inputSize = AllSize-readSize;
			printf("============readSize = %d inputSize =%d AllSize=%d\n",readSize,inputSize,AllSize);
			eixt=0;	
		}
		readSize +=inputSize;
		memcpy(inputBuf,data+readSize,inputSize);	
		for (i = 0; i < inputSize/2; i++) {
			const uint8_t* in = &inputBuf[2*channels*i];
			buf[i] = in[0] | (in[1] << 8);
		}
		n = E_IF_encode(amr, mode, buf, outbuf, dtx);
		fwrite(outbuf, 1, n, out);
		memcpy(outdata+outsize,outbuf,n);
		outsize+=n;
	}
	*size=outsize;
	free(inputBuf);
	fclose(out);
	E_IF_exit(amr);
	return 0;
}
#ifdef MAIN_AMR_16K
int main(int argc, char *argv[]) {

	return 0;
}
#endif
