# Introduction to Zephyr

Welcome to the Introduction to Zephyr course! You will find all of the example projects and solution code for the example projects in this repository. Follow the [Development Environment](#development-environment) and [Getting Started](#getting-started) sections below to set up the Zephyr and ESP32 toolchain 

## Development Environment

This is a development environment for creating Docker images with the Zephyr toolchain used to build source code for the ESP32. You build the image for your desired toolchain, store projects in the *workspace/* directory, and then run the image whenever you want to build (e.g. `west build`) the project. The intention is to use this environment as your VS Code working directory, but it is usable outside of VS Code.

![Screen Blink Build](.images/screen-blink-build.png)

> **Note**: the instructions below were verified with Python 3.12 running on the host system. If one of the *pip install* steps fails, try installing exactly Python 3.12 and running the command again with `python3.12 -m pip install ...`

You have a few options for using this development environment:

 1. (Default) The container runs *code-server* so that you can connect to `localhost:8800` via a browser to get a pre-made VS Code instance
 2. Run the image. In your local VS Code, install the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers). Connect to the running container. Select *File > Open Workspace from File...* and select the */zephyr.code-workspace* file when prompted.
 3. Override the image's *entrypoint* to get an interactive shell to run editing programs (e.g. `vim`, `mcedit`) and build (e.g. `west build`)

## Getting Started

Before you start, install the following programs on your computer:

 * (Windows) [WSL 2](https://learn.microsoft.com/en-us/windows/wsl/install)
 * [Docker Desktop](https://www.docker.com/products/docker-desktop/)
 * [Python](https://www.python.org/downloads/)

Windows users will likely need to install the [virtual COM port (VCP) drivers from SiLabs](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads).

### Install Dependencies

Open a terminal, navigate to this directory, and install the following dependencies:

Linux/macOS:

```sh
python -m venv venv
source venv/bin/activate
python -m pip install pyserial==3.5 esptool==4.8.1
```

Windows (PowerShell):

```bat
Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy Unrestricted -Force
python -m venv venv
venv\Scripts\activate
python -m pip install pyserial==3.5 esptool==4.8.1
```

From this directory, build the image (this will take some time):

```sh
docker build -t env-zephyr-espressif -f Dockerfile.espressif .
```

You can ignore the warning about setting the password as an `ARG` in the Dockerfile. The container is fairly unsecure anyway; I only recommend running it locally when you need it. You will need to change the password and configure *code-server* and *sshd* to be more secure if you want to use it remotely.

Run the image in *VS Code Server* mode. Note that it mounts the local *workspace/* directory into the container! We also expose ports 3333 (OpenOCD), 2222 (mapped from 22 within the container for SSH), and 8800 (*code-server*).

Linux/macOS:

```sh
docker run --rm -it -p 3333:3333 -p 2222:22 -p 8800:8800 -v "$(pwd)"/workspace:/workspace -w /workspace env-zephyr-espressif
```

Windows (PowerShell):

```bat
docker run --rm -it -p 3333:3333 -p 2222:22 -p 8800:8800 -v "${PWD}\workspace:/workspace" -w /workspace env-zephyr-espressif
```

Alternatively, you can run the image in interactive mode by adding the `--entrypoint /bin/bash` argument. This will allow you to skip running the VS Code server in the background.

### Connect to Container

With the Docker image built, you have a few options to connect to the development environment: browser, Dev Containers, SSH. Choose one of the options below.

#### Option 1: Connect via Browser

Open a browser and navigate to http://localhost:8800/.

> **Important!** Take note of the two directories in your VS Code instance:
> * ***/workspace*** is the shared directory between your host and container.
> * ***/opt/toolchains/zephyr*** is the Zephyr RTOS source code. It is for reference only and should not be modified!

#### Option 2: VS Code Dev Containers

Dev Containers is a wonderful extension for letting you connect your local VS Code to a Docker container. Feel free to read the [official documentation](https://code.visualstudio.com/docs/devcontainers/containers) to learn more.

In your local VS Code, install the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension.

Open the command palette (Ctrl+Shift+P) and search for **Dev Containers: Attach to Running Container**. Click it, and you should see a container of your *env-zephyr-espressif* image running. Click the container from the list. A new VS Code window will open and install the required dependencies.

Go to **File > Open Workspace from File..** and select the **/zephyr.code-workspace** file when prompted. Enter the password again if requested. This should configure your VS Code workspace with the */workspace* directory mapped from the host along with */opt/toolchains/zephyr* and */opt/toolchains/modules* so you can browse the Zephyr RTOS source files.

#### Option 3: VS Code SSH

If you want to develop Zephyr applications using your local instance of VS Code, you can connect to the running container using SSH. This will allow you to use your custom themes, extensions, settings, etc.

In your local VS Code, install the [Remote - SSH extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-ssh).

Open the extension in VS Code and create a new connection: **root@localhost:2222**.

Connect and login using the password in the Dockerfile (default: `zephyr`). Go to **File > Open Workspace from File..** and select the **/zephyr.code-workspace** file when prompted. Enter the password again if requested. This should configure your VS Code workspace with the */workspace* directory mapped from the host along with */opt/toolchains/zephyr* and */opt/toolchains/modules* so you can browse the Zephyr RTOS source files.

### Recommended Extensions

I recommend installing the following VS Code extensions to make working with Zephyr easier (e.g. IntelliSense). Note that the *zephyr.code-worspace* file will automatically recommend them.

 * [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)AZ
 * [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
 * [nRF DeviceTree](https://marketplace.visualstudio.com/items?itemName=nordic-semiconductor.nrf-devicetree)
 * [Microsoft Hex Editor](https://marketplace.visualstudio.com/items?itemName=ms-vscode.hexeditor)

### Build Demo Application

Open a terminal in the VS Code client and build the project. Note that I'm using the [ESP32-S3-DevKitC](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html) as my target board. Feel free to change it to one of the [other ESP32 dev boards](https://docs.zephyrproject.org/latest/boards/index.html#vendor=espressif).

```
cd apps/01_blink
west build -p always -b esp32s3_devkitc/esp32s3/procpu -- -DDTC_OVERLAY_FILE=boards/esp32s3_devkitc.overlay
```

With some luck, the *blink* sample should build. The binary files will be in *workspace/apps/blink/build/zephyr*, which you can flash using [esptool](https://docs.espressif.com/projects/esptool/en/latest/esp32/).

### Flash Demo Application

Connect a USB cable from your computer to the **UART port** on the development board. If you do not see the serial/COM port on your host OS, you might need to [install the necessary SiLabs VCP driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads). In a new terminal on your **host computer**, activate the Python virtual environment (Linux/macOS: `source venv/bin/activate`, Windows: `venv\Scripts\activate`) if not done so already.

Flash the binary to your board. For some ESP32 boards, you need to put it into bootloader by holding the *BOOTSEL* button and pressing the *RESET* button (or cycling power). Change `<PORT>` to the serial port for your ESP32 board (e.g. `/dev/ttyS0` for Linux, `/dev/tty.usbserial-1420` for macOS, `COM7` for Windows). You might also need to install a serial port driver, depending on the particular board.

> **Important!** make sure to execute flashing and serial monitor programs from your **host OS** (not from within the Docker container)

```sh
python -m esptool --port "<PORT>" --chip auto --baud 921600 --before default_reset --after hard_reset write_flash -u --flash_mode keep --flash_freq 40m --flash_size detect 0x0 workspace/apps/01_blink/build/zephyr/zephyr.bin
```

> **Important**: If you are using Linux and get a `Permission denied` or `Port doesn't exist` error when flashing, you likely need to add your user to the *dialout* group with the following command: `sudo usermod -a -G dialout $USER`. Log out and log back in (or restart). You should then be able to call the *esptool* command again to flash the firmware.

Open a serial port for debugging. Change `<PORT>` to the serial port for your ESP32 board.

```sh
python -m serial.tools.miniterm "<PORT>" 115200
```

You should see the LED state printed to the console. Exit with *ctrl+]* (or *cmd+]* for macOS).

## License

All software in this repository, unless otherwise noted, is licensed under the [Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) license.
