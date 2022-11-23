# playdate-c-game-template
Boilerplate C project for the Playdate console.

Going to keep improving on this until it has a little sample of all the main features of PdSDK.

Todo:
 - Sound
 - Key presses
 - Accelerometer
 - Sprite (animated using spritesheet)
 - Custom template card images/animations set with imagePath - https://sdk.play.date/1.12.3/Inside%20Playdate.html#pdxinfo

I currently use Visual Studio 2022 for building and cmake. 
I use this because I use VS debugging tools for C projects anyway but could change in the future.
I am aware that I can just use pdc to build but then I can't use VS C debugging features like watch window, memory window and disassemblies.

## .vscode/settings.json 
There is an argument for when the cmd Terminal is started to initialise Visual Studio vars which include cmake.
Change this path to be wherever your repo is located e.g:
```
"Command Prompt": {
  "path": [
      "${env:windir}\\Sysnative\\cmd.exe",
      "${env:windir}\\System32\\cmd.exe"
  ],
  "args": ["/k","P:\\playdate-c-game-template\\dev_env\\shell.bat"],
  "icon": "terminal-cmd"
},
```

## dev_env/shell.bat
Again, change the paths to point to your repo e.g:
```
set path=<pathToYourRepo>\dev_env;<pathToYourRepo>\src;<pathToYourPlaydateSDKFolder>\bin;%path%
```

## Build instructions
1. Create build directory in your project root.
2. `cd build.`
3. `cmake ..`
4. Ensure cmake is installed first and ensure that you ran dev_env/shell.bat (sometimes vscode does run it when reopening my project folder).
