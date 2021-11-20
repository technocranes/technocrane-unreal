# Technocrane Unreal Engine Plugin

[Technocrane Tracker Plugin on an official marketplace](https://unrealengine.com/marketplace/en-US/slug/technocrane-tracker?fbclid=IwAR0Fj0Ma3GsJ5hbG-oYZzfJeObZsdd8iSZKVrDtZPjCpr2IUWdOS83wVVsA)

[Introduction Video](https://youtu.be/VPmdLAun9WQ)

[<img src="https://discord.com/assets/ff41b628a47ef3141164bfedb04fb220.png" width=10% height=10%>](https://discord.gg/Uqe2UQxq)

# How to Install Manually

  Create a "Plugins" directory in the project root (near the "Content" directory) and create there sub-folder "TechnocranePlugin". Put the repository folders into the "TechnocranePlugin" folder. (e.g. "/MyProject/Plugins/TechnocranePlugin")

  If you don't have development environment to compile the plugin, you can download precompiled binaries from a release section of the repository.

  In Unreal Editor first of all you should activate Live Link plugin if you don't have that done yet.
  [![Step1](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_1.jpg)]()

  When you are opening project for a first time, you have to go to Settings->Plugins, find there Technocrane Plugin and Enable it.
[![TechnocranePlugin](https://github.com/technocranes/technocrane-unreal/blob/master/Images/TechnocranePlugin.JPG)]()

  Next step will be to open a live link hub
[![Step2](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_2.jpg)]()  

  In the Live Link window you should add a new source. Should it be a network connection or connection by using a serial port? You should specify some connection settings there. The values are based on default settings for Technocrane plugin.
[![Setup3](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_3.jpg)]()

 Then Source has been added and you see Camera subject with green status of receving data, then you can connect that to any cinematic camera in the scene. Add Live Link Component Controller to a cinematic camera.
[![Step4](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_4.jpg)]()

 Default settings and camera frames rate are presented in project settings under technocrane tracker group
[![Settings](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_6.jpg)]()

# Live Link Frame Meta Data and Properties

Every Frame Data constains additional meta data
* CameraOn - 1/0 values
* Running - 1/0 values
* IsZoomCalibrated - 1/0 values
* IsFocusCalibrated - 1/0 values
* IsIrisCalibrated - 1/0 values
* PacketNumber - number of a packet
* HasTimeCode - packet data recevies time code, 1/0
* RawTimeCode - raw packet timecode value
* FrameRate - string of camera frame rate (according to technocrane project settings)

Every Frame Data contains property values
* TrackPosition
* PacketNumber
* raw x
* raw y
* raw z
* raw pan
* raw tilt
* raw roll
* CameraOn
* Running

[![FrameDataPrint](https://github.com/technocranes/technocrane-unreal/blob/master/Images/frame_data_print.jpg)]()


# Video Tutorial

[![plugin_introduction](https://youtu.be/Nxp08jvDGdk)](https://youtu.be/Nxp08jvDGdk)

# Contact

Please post issues and feature requests to this [github repository issues section](https://github.com/technocranes/technocrane-unreal/issues)
