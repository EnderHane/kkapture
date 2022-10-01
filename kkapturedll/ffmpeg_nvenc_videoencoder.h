#pragma once

#ifndef __FFMPEG_NVENC_VIDEOENCODER_H__
#define __FFMPEG_NVENC_VIDEOENCODER_H__

#include "bmp_videoencoder.h"

class FFMPEGNVENCVideoEncoder : public BMPVideoEncoder
{
	int fpsNum, fpsDenom;
	char opts[256];
	char x264ExePath[MAX_PATH];

	HANDLE hProcess;
	HANDLE hStream;

public:
	FFMPEGNVENCVideoEncoder(const char* fileName, int fpsNum, int fpsDenom, const char* opts);
	virtual ~FFMPEGNVENCVideoEncoder();
	virtual void WriteFrame(const unsigned char* buffer);
};

#endif