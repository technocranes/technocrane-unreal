# Technocrane Unreal Engine Plugin

[Technocrane Tracker Plugin on an official marketplace](https://unrealengine.com/marketplace/en-US/slug/technocrane-tracker?fbclid=IwAR0Fj0Ma3GsJ5hbG-oYZzfJeObZsdd8iSZKVrDtZPjCpr2IUWdOS83wVVsA)

# How to Install Manually

  Create a "Plugins" directory in the project root (near the "Content" directory) and create there sub-folder "TechnocranePlugin". Put the repository folders into the "TechnocranePlugin" folder. (e.g. "/MyProject/Plugins/TechnocranePlugin")

  If you don't have development environment to compile the plugin, you can download precompiled binaries from a release section of the repository.

  In Unreal Editor first of all you should activate Live Link plugin if you don't have that done yet.
  [![Step1](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_1.JPG)]()

  When you are opening project for a first time, you have to go to Settings->Plugins, find there Technocrane Plugin and Enable it.
[![TechnocranePlugin](https://github.com/technocranes/technocrane-unreal/blob/master/Images/TechnocranePlugin.JPG)]()

  Next step will be to open a live link hub
[![Step2](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_2.JPG)]()  

  In the Live Link window you should add a new source. Should it be a network connection or connection by using a serial port? You should specify some connection settings there. The values are based on default settings for Technocrane plugin.
[![Setup3](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_3.JPG)]()

 Then Source has been added and you see Camera subject with green status of receving data, then you can connect that to any cinematic camera in the scene. Add Live Link Component Controller to a cinematic camera.
[![Step4](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_4.JPG)]()

 Default settings and camera frames rate are presented in project settings under technocrane tracker group
[![Settings](https://github.com/technocranes/technocrane-unreal/blob/master/Images/setup_6.JPG)]()

# Video Tutorial

[![plugin_introduction](https://youtu.be/Nxp08jvDGdk)](https://youtu.be/Nxp08jvDGdk)

# Contact

Please post issues and feature requests to this [github repository issues section](https://github.com/technocranes/technocrane-unreal/issues)
