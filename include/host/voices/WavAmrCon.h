#ifndef _WAVAMRCON_H
#define _WAVAMRCON_H

#ifdef __cplusplus
extern "C" {
#endif

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
extern struct wave_pcm_hdr pcmwavhdr;

/*******************************************************
函数功能:将wav格式文件编码成amr格式数据  8k 采样率,单声道
参数:	wavsrc wav格式数据内容 amrsrc amr格式数据内容 a_size数据大小
返回值: -1 编码失败 0 编码成功
********************************************************/
extern int WavToAmr8k(const char *wavsrc,char *Outdata,int *OutSize);
/*******************************************************
函数功能:将wav格式文件编码成amr格式文件
参数:	infile 输入文件路径 outfile 输出文件路径
返回值: -1 编码失败 0 编码成功
********************************************************/
extern int WavToAmr8kFile(const char *infile,const char *outfile);

/*******************************************************
函数功能:将Amr格式文件编码成Wav格式文件
参数:	infile 输入文件路径 outfile 输出文件路径
返回值: -1 编码失败 0 编码成功
********************************************************/
extern int AmrToWav8k(const char *infile,const char *outfile);

#ifdef __cplusplus
}
#endif

#endif
