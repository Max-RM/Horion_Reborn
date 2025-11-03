Horion is a Minecraft: Bedrock Edition utility mod designed to enhance gameplay. We do not take responsibility in anything done with this utility mod.
If you want to contribute to this project, feel free to fork this repository and submit a pull request.

Original repo: https://github.com/horionclient/Horion

## Description
If you tried to use Horion on an older version of Minecraft Bedrock Edition, you probably ran into the problem of "Download a new injector ..." or the crash of the game itself or the inability to import the DLL itself.
This is because the DLL code itself has a built-in mechanism for checking the new version on Horion servers, and if a newer version is detected, it prevents you from playing.
But @dmitryi365 found a workaround method that apparently works on all versions of the game, after which you will finally be able to use Horion on the old version of the game without problems.
Manual is here:
Find the file: "Loader.cpp"
In the folder:
"<SOURCE_CODE>\Horion\"
And use a text editor to find and replace the line:
```
bool isConnected = horionToInjector->isPresent && injectorToHorion->isPresent && horionToInjector->ProtocolVersion >= injectorToHorion->ProtocolVersion;
```
with:
```
bool isConnected = true;
```
Then find and replace the string:
```
g_Data.setInjectorConnectionActive(isConnected);
```
With:
```
g_Data.setInjectorConnectionActive(true);
```
Then find and the string:
```
CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)injectorConnectionThread, lpParam, NULL, &CONTREAD);
```
And add "//" at the very beginning to comment it, that is:
```
//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)injectorConnectionThread, lpParam, NULL, &contread);
```
Then find the lines:
```
	while (!g_Data.isInjectorConnectionActive()) {
		Sleep(10);
		if (!isRunning)
			ExitThread(0);
	}
```
and comment them like this:
```
	/*while (!g_Data.isInjectorConnectionActive()) {
		Sleep(10);
		if (!isRunning)
			ExitThread(0);
	}*/
```
After that, the work with this file will be completed, save and close.
Then find the file:
"GameData.h"
In the folder:
"<SOURCE_CODE>\Memory\"
And use a text editor to find and replace the single line:
```
bool injectorConnectionActive = false;
```
With:
```
bool injectorConnectionActive = true;
```
That is, simply change false to true.
After that, the work with this file will be completed, save and close.
This is the whole method. Tested on 1.16.20.03 and 1.16.40.02 and worked without any problems.
When opening in Visual Studio, if it prompts you to update the SDK version and the toolset, then agree, as this does not cause errors.

When compiling in Visual Studio, if it prompts you to update the SDK version and the toolkit, then agree, as this does not cause errors.
Before compiling, be sure to change the type from "Debug" to: "Release", a "Unable to start" error may occur after compilation, then ignore it, as it is the .DLL that is not .exe and not supposed to be running.
Instead of Horion Injector use this one to import .DLL and after launch the game click "Inject":
https://github.com/fligger/FateInjector/releases

If you have any questions ask here: https://github.com/Max-RM/Horion_Reborn/issues


## Licensing
This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
You are allowed to fork the project and distribute it, if you give credit. You may not sell any code protected by the license.
Not all source files are protected by this license - Some third party libraries (Chakra, DX11) may be under different copyright.
