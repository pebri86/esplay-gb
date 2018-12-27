#ifndef AUDIO_H
#define AUDIO_H

#define I2S_NUM     I2S_NUM_0

void audio_init(int sample_rate);
void audio_submit(short* stereoAudioBuffer, int frameCount);
void audio_terminate();

#endif
