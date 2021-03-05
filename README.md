## scat

Very simple **linux** command-line utility to print files with syntax highlighting. Just like `cat`, but with colors.

![GIF Demo](https://i.imgur.com/5fHVYNt.gif)

# Installation
`scat` can be installed with either of the following ways
1) Build from source, then execute an `install.sh` script
2) Install the binary, then execute a similar `install.sh` script

### 1. Building Source
The source can be built with a verison of`g++` that supports `c++17`. The default that comes 
with `apt-get` should be fine.

```sh
sudo apt install g++
```
aditionally the `sqlite3` library files are required
```sh
sudo apt-get install libsqlite3-dev
```
then you can clone and execute the `install.sh` script which does some minor configurations

```sh
git clone https://github.com/Greece4ever/scat.git
cd scat
. ./install.sh
```
Then, if everything goes right, there should be an executable in the same directory called `scat`.

Optionaly you can then move it to `/usr/bin`
```sh
sudo mv scat /usr/bin # or better /usr/local/bin
```
### 2. Installing built binary (Alternative)

Installing the already built binary also requires some configuration.

Install and extract the zip from the [tag release](https://github.com/Greece4ever/scat/releases/download/0.0.0/scat.zip). Assuming `curl` and `unzip` are installed.
```
curl --output scat.zip -L -get https://github.com/Greece4ever/scat/releases/download/0.0.0/scat.zip
unzip scat.zip > /dev/null
```
Then execute the `install.sh` script
```sh
# cd scat # you may have to do this if you install via browser
cd build 
. ./install.sh
sudo mv ./scat /usr/bin # or /usr/local/bin
```
if you've moved to `/usr/bin/` or `/usr/local/bin` You can safely remove the source files with `cd .. && rm -rf ./scat`.

# Usage
Assuming you've run the install script and everything went smoothly.
```sh
scat ./file # Print file with syntax highlithing
```
This will use the default syntax highlighting that came with with repository.

To add your own
```sh
scat --parse --save file.lang # print all the colors and save
scat --save file.lang  # simply save
```

Were a .lang file has the following format

```
.cpp

KWD:
    int BLUE
    float BLUE
RPT:
    */   /*   GREY
    //    \n  GREY
    "     "   GREEN
```
The first thing must be the file extension (`.cpp` here). `KWD` stands for keyword and it represent language keywords (e.g `def` in python) and `RPT` stands for repeating keywords, which is for things like string quotes (`"`, `'`) where everything between them is Green. Other example files can be found [here](https://github.com/Greece4ever/scat/tree/master/langs).

If no syntax highlithing is found for a specific format, the default will be used (which is set in the `install.sh` script) and it can be set via.
```sh
scat --default ./default.lang
```
Languages who have similar (or the same) syntax highlithing rules don't have to be `--saved` again. If you already have `--save`ed for example `.py` you can do this for `.pyc` files

```sh
scat --create_ptr .py .pyc # .pyc points to .py
```
