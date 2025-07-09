<h1 align="center">Vasciipp</h1>

<h3 align="center">Vasciipp is a real-time video-to-ascii player for Linux written in C/C++ using <a href="https://opencv.org/">OpenCV</a></h3>

## Showcase
[![Demo](https://img.youtube.com/vi/aKAY3BNQ9qs/hqdefault.jpg)](https://www.youtube.com/watch?v=aKAY3BNQ9qs)

## Features
- Dynamic Scaling
- Customizable Character Set
- Basic Colors Options
- No Trailing Line

## Tips & Tricks
- Zoom out / decrease font size for higher resolutions
- Software flow control allows to pause with `ctrl + s` and resume with `ctrl + q`
- Use `--loop` for GIFs
> [!NOTE]
> GIFs may playback slightly faster due to different per-frame delays
- Use `echo -e "\033[38;2;200;69;69;48;2;30;0;50m" && ./vasciipp /path/to/file`
or `echo -e "\033[38;5;125;48;5;233m" && ./vasciipp /path/to/file`
for a much richer color pallete as opposed to the limited option mappings.
[colors cheatsheet](https://gist.github.com/ConnerWill/d4b6c776b509add763e17f9f113fd25b#256-colors)

> [!Warning]
>CPU utilization is heavily tied to the files resolution and framerate. Consider downscaling if needed.

## Usage

`./vasciipp <file> <option(s)>`

quit with `q` or interrupt `ctrl + c`
>[!NOTE]
If the program does not exit cleanly it can result in invisible input or cursor, type reset and hit enter or open a new terminal instance to make sure it is fully functional.

## Dependencies
- [OpenCV](https://opencv.org/) >= 4.6.0 and must be compiled with FFmpeg support
#### NixOS
`nix-shell -p opencv` or `nix-shell` to use [shell.nix](shell.nix)

or 

```nix
environment.systemPackages = [
  pkgs.opencv
];
```
#### Ubuntu / Debian
```bash
sudo apt update
sudo apt install libopencv-dev -y
```
#### Arch / Manjaro
```bash
sudo pacman -Syu
sudo pacman -S opencv
```

## Building

```bash
git clone https://github.com/Mythicsoul/Vasciipp.git
cd Vasciipp
mkdir build
cmake . -B build
cmake --build build
```
## Nix Flake

Try it with:
```nix
nix run github:mythicsoul/vasciipp
```
Or add it to your flakes inputs
```nix
inputs.vasciipp.url = "github:mythicsoul/vasciipp";
```
and then use it in your configuration
```nix
environment.systemPackages = [
  inputs.vasciipp.packages.${pkgs.system}.default
];
```

