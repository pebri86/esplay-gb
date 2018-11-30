#pragma once

void audio_init(int sample_rate);
void audio_terminate();
void audio_submit(short* stereoAudioBuffer, int frameCount);
int audio_sample_rate_get()
