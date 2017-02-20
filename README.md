#Quick Curver
This is an OpenGL accelerated Qt/C++ implementation of the famous game "Achtung die Kurve".

![alt tag](http://i.imgur.com/IGa3NwA.png)

Please note that this project is yet in development and hasn't reached a stable version for now. This means that you could experience bugs, when playing this game.

#Features
* Material Design
* Local Multiplayer
* Online Multiplayer (still in development)
* Bots
* Items


#Installation
##Compiling from source
Quick Curver uses [qml-material](https://github.com/papyros/qml-material) which must be installed first:
Navigate to a directory where you want to install qml-material and execute:
```{r, engine='bash', count_lines}
git clone git@github.com:papyros/qml-material.git #you can alternatively clone using HTTPS
cd qml-material; mkdir build; cd build
qmake ..
make
make install # use sudo if necessary
```
Now we can proceed with the installation of Quick Curver itself:
Navigate to a new directory where you want to install Quick Curver and execute:
```{r, engine='bash', count_lines}
git clone git@github.com:Magnus2552/quickcurver.git #you can alternatively clone using HTTPS
cd quickcurver; mkdir build; cd build
qmake ..
make
./QuickCurver #starting the executable may vary on OSs other than Linux
```
If it fails to execute, make sure that it has execute permissions (chmod +x ./QuickCurver)

#Multiplayer
To play multiplayer, the host has to make at least one player an "Online Player" by clicking on that player in the Playerselector screen and using the drop down menu. Once a slot is open to join, the host has to publish his IP to the client. Note that the program automatically prints out the IP, assuming you have started it CLI. If you are not in the same local network, the host has to use [Port Forwarding](https://en.wikipedia.org/wiki/Port_forwarding) to make his device available to the internet. When the client receives the host's IP, he has to start the program, click on the "Join" Icon on the top right and enter the IP. If the join request was successfull, you will see "Joined, waiting for host to start...", otherwise the connection will timeout after a few seconds.

#Troubleshooting
##The drawing does not get updated, or only rarely
Use one of the following commands to start QuickCurver instead:
```{r, engine='bash', count_lines}
QSG_RENDER_LOOP=basic ./QuickCurver #prefer this one
QSG_RENDER_LOOP=windows ./QuickCurver #alternatively use this command
```
