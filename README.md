# :warning: Archived :warning:
now at https://invent.kde.org/plata/telly-skout

# telly-skout
A convergent EPG based on Kirigami.

## Build
### Dependencies
#### Manjaro
```
sudo pacman -Syu cmake extra-cmake-modules gcc git make
```

### Clone and Build
```
git clone https://github.com/plata/telly-skout.git
cd telly-skout
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Install
```
sudo make install
```

## Run
```
telly-skout
```
