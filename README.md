# Tor-PIR-do
Security, Latency, and Throughput of PIR and the Tor Network

For more information about this project, please see our paper.

## Compiling and Installing
The following instructions got everything up-and-running nicely on our Amazon EC2 Ubuntu instance.

### General

First, install all the dependencies you can with a nice package manager.

```
sudo apt-get update
sudo apt-get cmake build-essential g++ cmake python-dev autotools-dev libicu-dev build-essential libbz2-dev
```

Now begins the slog of installing dependencies from source. First up is SEAL.

### SEAL  v3.2.0

```
git clone --single-branch --branch 3.2.0 https://github.com/microsoft/SEAL.git
cd SEAL/native/src
cmake .
make
sudo make install
cd ../..
```

### Boost v1.71.0

Note, that at the time of writing this, Boost v1.71.0 could not be installed with `apt-get` and could only be built from source.

```
wget https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.gz -O boost_1_71_0.tar.gz
tar xzvf boost_1_71_0.tar.gz
cd boost_1_71_0
./bootstrap.sh --prefix=/usr/local
./b2
sudo ./b2 install
```

### Your's Truly

Note: in the CMakeLists.txt file, you should edit the path to your `g++` compiler.

```
git clone https://github.com/ajgaidis/tor-pir.git
cd tor-pir
cmake .
make
```

After this last step, you should be all good to go. The binaries you just made are located in `./bin`. In order to see a help menu run `./bin/server --help` or `./bin/client --help`.