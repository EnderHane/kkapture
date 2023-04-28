$script_dir = Split-Path -Parent $MyInvocation.MyCommand.Definition
New-Item -Path (Join-Path -Path $script_dir -ChildPath "publish") -ItemType Directory -Force
New-Item -Path (Join-Path -Path $script_dir -ChildPath "publish\tools") -ItemType Directory -Force
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "build\bin\Win32\Release\*.exe") -Destination (Join-Path -Path $script_dir -ChildPath "publish") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "build\bin\Win32\Release\*.dll") -Destination (Join-Path -Path $script_dir -ChildPath "publish") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "build\bin\x64\Release\*.exe") -Destination (Join-Path -Path $script_dir -ChildPath "publish") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "build\bin\x64\Release\*.dll") -Destination (Join-Path -Path $script_dir -ChildPath "publish") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "*") -Include *.txt -Destination (Join-Path -Path $script_dir -ChildPath "publish") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "*") -Include *.md -Destination (Join-Path -Path $script_dir -ChildPath "publish") -Force -Recurse
Copy-Item -Path (Join-Path -Path $script_dir -ChildPath "tools\*") -Exclude *.zip -Destination (Join-Path -Path $script_dir -ChildPath "publish\tools") -Force -Recurse
Expand-Archive -Path (Join-Path -Path $script_dir -ChildPath "tools\ffmpeg.zip") -DestinationPath (Join-Path -Path $script_dir -ChildPath "publish\tools") -Force
Expand-Archive -Path (Join-Path -Path $script_dir -ChildPath "tools\x264.zip") -DestinationPath (Join-Path -Path $script_dir -ChildPath "publish\tools") -Force
