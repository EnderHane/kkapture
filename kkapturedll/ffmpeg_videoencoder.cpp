#include "stdafx.h"
#include "ffmpeg_videoencoder.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

FFmpegVideoEncoder::FFmpegVideoEncoder(const char* fileName, int _fpsNum, int _fpsDenom, const char* _opts, const char* _in_opts)
    : BMPVideoEncoder(fileName)
{
    fpsNum = _fpsNum;
    fpsDenom = _fpsDenom;
    strncpy_s(out_opts, 256, _opts, _TRUNCATE);
    strncpy_s(in_opts, 256, _in_opts, _TRUNCATE);
    hProcess = hStream = 0;

    TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
    GetModuleFileName((HINSTANCE)&__ImageBase, x264ExePath, _countof(x264ExePath));
    _tsplitpath(x264ExePath, drive, dir, fname, ext);
    strcat_s(dir, "tools");
    _tmakepath(x264ExePath, drive, dir, _T("ffmpeg"), _T("exe"));
}

FFmpegVideoEncoder::~FFmpegVideoEncoder()
{
    if (hStream)
    {
        CloseHandle(hStream);
    }
    if (hProcess)
    {
        Real_WaitForSingleObject(hProcess, INFINITE);
    }
}

void FFmpegVideoEncoder::WriteFrame(const unsigned char* buffer)
{
    DWORD dwDummy;

    if (!xRes || !yRes)
    {
        return;  // resolution not set yet, can't do anything
    }

    // start ffmpeg
    if (!hProcess && !hStream)
    {
        char temp[512];
        static const char eol[2] = { 13, 10 };

        // create a SECURITY_ATTRIBUTES structure with inheritance enabled (we need that twice)
        SECURITY_ATTRIBUTES sa;
        ZeroMemory(&sa, sizeof(sa));
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;

        // create log file name and open log file
        _snprintf_s(temp, sizeof(temp) / sizeof(*temp), "%s.ffmpeg.log", prefix);
        HANDLE hLogFile = CreateFile(temp, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        HANDLE hErrLog = 0;
        DuplicateHandle(GetCurrentProcess(), hLogFile, GetCurrentProcess(), &hErrLog, 0, TRUE, DUPLICATE_SAME_ACCESS);

        // create the input pipe
        HANDLE hChildInput = 0, hDummy = 0;
        CreatePipe(&hChildInput, &hDummy, &sa, 0);
        // create a non-inheritable copy of the write end and close the other (inheritable) original
        DuplicateHandle(GetCurrentProcess(), hDummy, GetCurrentProcess(), &hStream, 0, FALSE, DUPLICATE_SAME_ACCESS);
        CloseHandle(hDummy);

        // build the ffmpeg command line
        _snprintf_s(temp, sizeof(temp) / sizeof(*temp),
            "\"%s\" -y -f rawvideo -pix_fmt bgr24 -s %dx%d -r %f %s -i - %s \"%s.mp4\"",
            x264ExePath, xRes, yRes, (float)fpsNum/fpsDenom, in_opts, out_opts, prefix);
        printLog("ffmpeg: command line: %s\n", temp);
        WriteFile(hLogFile, (LPCVOID)&temp[0], (DWORD)strlen(temp), &dwDummy, NULL);
        WriteFile(hLogFile, (LPCVOID)&eol[0], 2, &dwDummy, NULL);

        // create the ffmpeg process
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hChildInput;
        si.hStdOutput = hLogFile;
        si.hStdError = hErrLog;
        if (Real_CreateProcessA(NULL, temp, NULL, NULL, TRUE, DETACHED_PROCESS, NULL, NULL, &si, &pi))
        {
            hProcess = pi.hProcess;
        }
        else
        {
            printLog("ffmpeg: failed to start encoder (error code 0x%08X)\n", GetLastError());
        }

        // close obsolete handles
        CloseHandle(hChildInput);
        CloseHandle(hLogFile);
        CloseHandle(hErrLog);
    }

    if (hStream && hProcess)
    {
        for (int y = yRes - 1; y >= 0; --y)
        {
            WriteFile(hStream, (LPCVOID)&buffer[y * xRes * 3], xRes * 3, &dwDummy, NULL);
        }
    }

    frame++;
}