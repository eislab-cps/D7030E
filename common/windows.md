# Installation for Windows users

## 1. WSL & Docker Desktop

### 1.1 Installation
See download instructions [here](/README.md#1-install-prerequisites)

### 1.2 NetAnim
Open WSL in the the repo root

```powershell
wsl -d Ubuntu
```

Install necessary tools.

```bash
# WSL
sudo apt update
sudo apt install -y qtbase5-dev qttools5-dev-tools build-essential
```

Download, unzip, and compile NetAnim

```bash
# Download
mkdir netanim && cd netanim
curl -LO https://www.nsnam.org/tools/netanim/netanim-3.108.tar.bz2

# Unzip
tar xjf netanim-3.108.tar.bz2
cd netanim-3.108

# Compile
qmake NetAnim.pro
make -j"$(nproc)"
```

To start NetAnim simply run the following command in WSL while in the root of the repository.

```bash
# WSL
./netanim/netanim-3.108/NetAnim
```

### 1.3 (Optional) Create a start batch file

Create a file named `netanim.bat` with the content

```powershell
wsl -d Ubuntu ./netanim/netanim-3.108/NetAnim
```

Simply open the file with cmd when you want to start NetAnim.

## 2. ⚠️ Docker image
Build the docker image. ⚠️ This process takes very long and it's recommended to only run once.

```powershell
docker build -t ns3-3.40:latest .
```

## 3. Docker container
Make sure Docker Desktop is running on your computer. Then run the following command to start the container with ns3.

```powershell
docker compose up -d
```

This will start a container with the ns3 image with among other things a bind mount from root to `/work`. This means that all code changes you make in the repository will be available inside the docker container. This is useful for making quick changes to the code without having to copy it between Windows and the docker container.

## 4. Running
To compile and run a c++ file with ns3:
1. Make sure Docker Desktop is running.
2. Right click `run.ps1` and open with Windows PowerShell.
3. Choose the `.cc` file you want to run.
4. The script will ask for any arguments. Here you can put whatever and it will be appended to the run command. Some examples are `--distance=50` or `> /work/Lab-00-Introduction/submission/hello_cpp_output.txt`. (Do note that redirection `>` doesn't work if the folder doesn't exist.)
5. The file will be compiled and run using ns3.

If you want to enter the docker container with a shell (for example to copy netanim xml file into repo) run either

```powershell
# Attach to existing shell session
docker attach ns3dev
```

or

```powershell
# Create new shell session
docker exec -it ns3dev bash
```

# What is different?
There are two main changes:

## 1. Docker compose
I opted for docker compose instead of running a container using `make shell` to allow for better configuration control.

## 2. Exec folder & run.ps1
Instead of having to
1. Attach to docker
2. Navigate to repo
3. Copy C++ file to `$NS3_DIR/scratch`
4. Run `ns3 build`
5. Run `ns3 run scratch/file.cc`

for every file i wanted to automate this process.

By creating a bind mount from `./exec` to `$NS3_DIR/scratch/exec` with the file [CMakeLists.txt](/exec/CMakeLists.txt), whenever `ns3 build` runs, a general application called `exec` is created. This means that as long as there is only one `.cc` file inside `./exec`, you can run it using `ns3 run exec`. This allowed for a workflow where I don't have to remember the exact name of the module I want to run. 

The workflow becomes:

1. Copy C++ file into `./exec` (without having to attach to docker)
2. Run `ns3 build`
3. Run `ns3 run exec`

The [run.ps1](/run.ps1) file automates this further by asking which file to copy and then runs both build and exec. This way I never have to attach to docker (except when copying anim files) or start WSL (except for running NetAnim).