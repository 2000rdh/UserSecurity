Overview:

1. Purpose and Background
3. Progress
4. Instructions/How It Works

******************************

1. Purpose and Background

The purpose of this project is to produce a convincing user behavior simulator based on the behavior of a specific real user. This would allow the bypassing of user behavior authentication, which has implications in both offensive and defensive contexts.

For a defensive example, automated sandboxes that are looking to combat sandbox-evading malware can programmatically pose as a real user in order to trigger malware execution in viruses that are looking for real user behavior in the form of mouse movement and clicking. As sandboxers have begun to automate some of these basic behaviors, evasive malware has also begun to look for more sophisticated behaviors like scrolling, and this could only continue to escalate. If a sandbox is able to replicate users much more realistically while still maintaining their automation, they will be able to consistently fool malware.

In an offensive example, while Windows 10 Defender's tamper protection prevents malware from automatically shutting its antivirus, it does not prevent the user from manually opening up the settings and toggling tamper protection off themselves, and a program that could simulate this could potentially bypass Windows security altogether in an untraditional way.

In my project, I have created three programs: Track.c, Sim.c, and findimg.py. I am also working on implementing some variation of Microsoft's recommended bitmap screen capture source code, GDI_CapturingAnImage.cpp (which I do not take credit for). Track.c records user mouse movement behavior with timestamps and coordinates and saves it into a .txt file (which is formatted like a csv). Sim.c aims to simulate the user by analyzing the collected data and utilizing it within its own AI to move similarly to the user. Then, once it is ready to carry out some action, it would reach out to a server hosting findimg.py, send a bitmap capture of the entire screen to it over a TCP socket, and receive back the coordinates for the destination coordinate to move to and click on or scroll up/down near. findimg.py would achieve this using image recognition, and would return the appropriate coordinates and action to Sim.c. It is designed to dynamically react to the screenshot in such a way that, if something goes wrong in the predetermined path from beginning to end, it can figure out where it's at and proceed from there.

In my current implementation, I have attempted to take on the offensive functionality, which would also sufficiently prove the capability for defensive functionality, which only needs to perform enough to trick the malware without completing any specific goals.

2. Progress

This is not the finished, fully functional code for the project. I am presently working on properly implementing Microsoft's recommended code for bitmap screen capture in C++. Because this is not currently working, the C and Python components cannot communicate with each other. As a result, I have been manually setting points in the C for testing and hardcoding the images into the Python for recognition. The very first image in the Python is a .bmp whereas the rest are .jpg, but it will work with all .bmp files.

Track.c: Currently, the Track.c program is able to record mouse movement for a period of time and produce a .txt file formatted as: time,xcoordinate,ycoordinate\n. This allows the data to be passed into the Sim.c file so it can attempt to mimic the user's mouse movement. Currently, it runs until it receives a Ctrl-C/Ctrl-Z signal, rather than the time being predetermined.

Sim.c: Currently, the Sim.c program is able to simulate a user, though in this implementation it can only simulate a more generic user. This is because I am still refining the overall patterns and RNG for the random noise, and once this is finished I will be altering this algorithm so speed and noise are based on user movements. I will also be integrating occasional pauses based on the user's own frequency and average pause time. Once the GDI bitmap capabilities are fully functional, the cursor's destination coordinates and whether it should click or scroll will be based on what findimg.py returns (clicking and scrolling are not yet implemented as a result). There are minor issues regarding floating point/double rounding errors that I have resolved as best as possible by changing the way certain comparisons are made.

findimg.py: Currently, findimg.py has nearly all of its functionality except for receiving bitmaps, as it currently has no bitmaps to receive. This will be done using sockets. The program can successfully move through the entire path up to tamper protection, however the dynamic adjustment system needs further debugging. I am still determining whether the program can bypass UAC or whether UAC being turned off will be a requirement for full functionality of the offensive component. Once this is determined, I will be able to complete the remainder of the path beyond UAC.

3. Instructions/How It Works

NOTE: The source code is allowed to be altered as needed, especially Sim.c.

The C code can be compiled and run as an executable, and the python can be run in an IDE or through the command line. The C was developed in Visual Studio 2019, and though C++-adjacent tools were used, these parts are written entirely in C. The Python is written in Python 3.

Once Track.c starts running, it will track time elapsed and mouse coordinates (and nothing else) until the program is stopped with Ctrl-C or Ctrl-Z. Then it will produce mtr.txt in the same folder.

When Sim.c is run, it will attempt to read from mtr.txt and proceed if it is found. Then it will analyze the data by sorting it into vectors and pulling the relevant vector data to be parameterized. After the analysis, the simulator is put into action. In the current implementation, there are two notable parts that may need to be altered within the program. The first is the destination coordinate, which is hard coded in, as the simulator only presently undergoes one iteration. As of right now it goes to the coordinate that would be the center of the screen on a 1280 x 720 resolution computer. The second is the time limit on the program. In order to prevent the program from potentially indefinitely hijacking the cursor, the loop that runs the simulator is timed. The unit is in seconds, and in the line /if((stop.time - start.time) >= 5) {/, the number 5 represents a time limit of 5 seconds on the loop.

When findimg.py is run, it goes through the algorithm and will display each image containing a rectangle enclosing the found destination image and a dot indicating where the C program would be told to click. The bigger image is often labeled as the "received" variable, and the image being searched for is the "template" image. These can be changed for testing purposes.