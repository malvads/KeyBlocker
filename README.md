# KeyBlocker

<p align="center">
  <img src="assets/tray.png" alt="KeyBlocker" width="200">
</p>

Another random crap I created because random windows kept popping up while I was cleaning my keyboard. Honestly, I’d rather spend an hour building this tool than deal with that again.

KeyBlocker is a macOS utility that allows you to temporarily disable your keyboard input. This is useful for cleaning your keyboard, preventing accidental keystrokes from pets or children, or simply ensuring no input is registered during certain tasks.

## Features

- **Keyboard Blocking**: Intercepts and blocks all keyboard events.
- **System Tray Integration**: Easily toggle blocking from the macOS menu bar.
- **Logging**: Configurable logging levels (Info, Error, Debug) for troubleshooting.
- **Ease of Use**: Simple command-line interface and minimalist UI.

## Prerequisites

- **macOS**: This application is specifically designed for macOS using the Core Graphics and Cocoa frameworks.
- **Accessibility Permissions**: The app requires Accessibility permissions to intercept keyboard events. When you first run the app, you will be prompted to allow it in **System Settings > Privacy & Security > Accessibility**.

## Getting Started

### Build

You can build the project using the provided `Makefile`.

```bash
# Build the binary
make

# Create the application bundle (KeyBlocker.app)
make bundle

# Create a distribution DMG
make dmg
```

### Run

You can run the application directly from the binary or open the created bundle.

```bash
# Run the binary
./key_blocker

# With custom log level
./key_blocker --log-level debug
```

### Installing from DMG

1. Download the latest `KeyBlocker.dmg` from the [Releases](https://github.com/malvads/KeyBlocker/releases) page.
2. Open the `.dmg` file.
3. Drag `KeyBlocker.app` into your `/Applications` folder.
4. Launch the app and grant the required **Accessibility Permissions** when prompted.
5. Relaunch the app (if permissions were granted)


## Usage

![KeyBlocker](assets/example.png)

Once running, you will see a tray icon in your menu bar. 
- Click the icon to toggle the keyboard block on or off.
- The tray icon will change state to indicate whether the keyboard is currently blocked.

### Command Line Arguments

- `-v`, `--verbose`: Enable debug logging.
- `--log-level <level>`: Set the log level. Available levels: `debug`, `info`, `error`.

## Project Structure

- `main.c`: Entry point of the application.
- `keyboard.c` / `keyboard.h`: Core logic for keyboard event interception.
- `tray.m` / `tray.h`: macOS system tray (menu bar) implementation using Objective-C and Cocoa.
- `logger.c` / `logger.h`: Logging utility.
- `Makefile`: Build instructions.
- `Info.plist`: Metadata for the application bundle.
- `assets/`: Icons and images used by the application.

## License

This project is licensed under the Affero General Public License v3.0 - see the [LICENSE](LICENSE) file for details (if applicable).
