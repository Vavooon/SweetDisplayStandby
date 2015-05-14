# SweetDisplayStandby
*Tool which turns off monitor after inactivity interval in the proper way*

All monitors i had (HDMI-capable ones) was unable go into standby mode when Windows turned them.
My current one even shows "Check cable" message for few minutes and does not power on when i move my mouse.

So i decided to write a software which replaces default Windows monitor power management functionality.
You need to disable monitor shutdown in control panel at first.

The software uses DDC/CI protocol to communicate with monitors, so you need to connect your one via DVI/HDMI.

Features:
*  Detects all connected monitors
*  Turns off monitor(s) after specified inactivity interval and turns them on
*  Reduces backlight level before after smaller inactivity interval 
*  Turns monitor off before PC standby and turns on it back after power on/wake
*  Allows to turn off all monitor by one global hotkey


I used Qt and icon from Faenza icon pack :) 
(Please tell me i case i should write about OSS usage somehow else.)

It works only in Windows (7 and newer).

Usage:

Add executable file to Startup. You can also specify --hide option and it will run in background even without tray icon.
Use Ctrl+F12 (selectable) shortcut to display the window.
