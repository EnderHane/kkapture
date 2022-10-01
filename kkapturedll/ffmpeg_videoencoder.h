#pragma once

#ifndef __FFMPEG_NVENC_VIDEOENCODER_H__
#define __FFMPEG_NVENC_VIDEOENCODER_H__

#include "bmp_videoencoder.h"

class FFmpegVideoEncoder : public BMPVideoEncoder
{
	int fpsNum, fpsDenom;
	char in_opts[256];
	char out_opts[256];
	char x264ExePath[MAX_PATH];

	HANDLE hProcess;
	HANDLE hStream;

public:
	FFmpegVideoEncoder(const char* fileName, int fpsNum, int fpsDenom, const char* opts, const char* _in_opts);
	virtual ~FFmpegVideoEncoder();
	virtual void WriteFrame(const unsigned char* buffer);
};

#endif