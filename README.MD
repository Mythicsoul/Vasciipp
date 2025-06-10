# <center>Vasciipp</center>

### <center>Vasciipp is a real-time video-to-ascii player for Linux written in C/C++ using [OpenCV](https://opencv.org/)</center>

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
- The `--invert` options adds a few empty spaces at the end to make the default ascii character set ` .:-=+*#%@` look nicer.  To circumvent that you can abstein from using it and instead do `echo "your char set" | rev` to reverse, copy it and then use that with `--ascii` .

> [!Warning]
>CPU utilization is heavily tied to the files resolution and framerate. Consider downscaling if needed.

## Dependencies
- [OpenCV](https://opencv.org/)
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

## Usage

`./vasciipp  <file> <option(s)>`

quit with `q` or interrupt `ctrl + c`
>[!NOTE]
If it does not exit cleanly, which can result in invisible input or cursor, type reset and hit enter or open a new terminal instance to make sure it is fully functional.

