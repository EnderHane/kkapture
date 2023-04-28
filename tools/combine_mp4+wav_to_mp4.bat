"%~dp0ffmpeg" -y -i "%~1" -i "%~dpn1.wav" -c:v copy -c:a aac -b:a 320k "%~dpn1_with_audio.mp4"
@pause