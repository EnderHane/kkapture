$script_dir = Split-Path -Parent $MyInvocation.MyCommand.Definition
New-Item -Path (Join-Path -Path $script_dir -ChildPath "install") -ItemType Directory -Force
New-Item -Path (Join-Path -Path $script_dir -ChildPath "install\tools") -ItemType Directory -Force
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "build\bin\Win32\Release\*") -Destination (Join-Path -Path $script_dir -ChildPath "install") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "build\bin\x64\Release\*") -Destination (Join-Path -Path $script_dir -ChildPath "install") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "*") -Include *.txt -Destination (Join-Path -Path $script_dir -ChildPath "install") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "*") -Include *.md -Destination (Join-Path -Path $script_dir -ChildPath "install") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "tools\*") -Exclude *.zip -Destination (Join-Path -Path $script_dir -ChildPath "install\tools") -Force -Recurse
Expand-Archive -Path (Join-Path -Path $script_dir -ChildPath "tools\ffmpeg.zip") -OutputPath (Join-Path -Path $script_dir -ChildPath "install\tools") -Force
Expand-Archive -Path (Join-Path -Path $script_dir -ChildPath "tools\x264.zip") -OutputPath (Join-Path -Path $script_dir -ChildPath "install\tools") -Force
