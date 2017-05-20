#ifndef resexampleRate_H 
#define resexampleRate_H

extern int ResamplerState_Voices(int channel,const char *inputFile,int inRate,const char *outFile,int outRate);
extern int ResamplerState_wavfileVoices(const char *inputWavFile,const char *outwavFile,int outRate);

#endif
