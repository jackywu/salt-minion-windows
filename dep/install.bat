cmd.exe /c start "puppet-install" /w vcredist_x64.exe /q /norestart
msiexec.exe /qn /norestart /i python-2.7.9.amd64.msi
msiexec.exe /qn /norestart /i VCForPython27.msi
C:\Python27\python.exe ez_setup.py
C:\Python27\Scripts\easy_install.exe pip
Win64OpenSSL_Light-1_0_2a.exe
C:\Python27\Scripts\pip.exe install -r requirements.txt

echo "The dependencies are installed successfully"
