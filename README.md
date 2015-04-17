# salt-minion-windows
Compiled Binary of salt-minion for windows and related files to build that.

Because I wanna install salt-minion by puppet silently, but the official setup does not work as they said [here](http://docs.saltstack.com/en/latest/topics/installation/windows.html). The setup.exe fetch vcredist in the background and popup a 'confirm box' when
 download failed, although you use `Salt-Minion-0.17.0-Setup-amd64.exe /S /master=yoursaltmaster /minion-name=yourminionname` this command as they taught.

Some guys else also encounter the same problem

* <https://github.com/saltstack/salt/issues/18362>
* <https://github.com/saltstack/salt/issues/18935>
 
# What's the problem


After searching the code by 'VC redist package download', and you will find the file `./pkg/windows/installer/Salt-Minion-Setup.nsi` which is the spec file to build setup binary. And what's strange is the code of popup box are commented like [this](https://github.com/saltstack/salt/blob/2014.7/pkg/windows/installer/Salt-Minion-Setup.nsi#L184), but the binary [Salt-Minion-2014.1.7-AMD64-Setup.exe](https://docs.saltstack.com/downloads/Salt-Minion-2014.1.7-AMD64-Setup.exe) do the download in fact. So I suspect the saltstack team used a different nsi file to built the official binary. I decide compile the binary by myself to prove that. This is the [reference](http://docs.saltstack.com/en/latest/topics/installation/windows.html#setting-up-a-windows-build-environment).

# Process

Flow the reference and make a little modification.
see my 



