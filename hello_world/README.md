# GOALS
- setting up your environment for esp-idf
- running a simple program
- why bother?

## Setting up your environment

You can use the vscode/ecclipse extension and let that do all the manual work for you or if you are braindead like I am use a different editor like NeoVim you can use the manual installation process

Both are explained well on the esp-idf [documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#software).

If you are using NeoVim, autocomeplete will show up after you build the program once. To get rid off the -fno-... warning you can place the .clangd file that in your project directory.

## Running a simple program

The ESP32-DevKit does not require you to press any of flash or reset buttons to flash the program. 

But on some boards like the esp32-cam you need to press and hold the flash button before inserting the USB button and flash the program. Releaase the flash button once the logs tell you its writing the program at whatever address. After flashing the programming you need to press the reset to start running the program.

## Why bother?

The arduino is more than enough for simple programs but it abstracts away a lot of the internals of how the esp works. Using the idf gives you better control over running concurrent tasks on both cores. Which will be illustrated in the next example.