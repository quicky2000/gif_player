Gif_player
==========

SDL2 based software displaying animated GIFs

Continuous integration with [Travis-Ci](https://travis-ci.com/quicky2000/gif_player) : ![Build Status](https://travis-ci.com/quicky2000/gif_player.svg?branch=master)

License
-------
Please see [LICENSE](LICENSE) for info on the license.

Build
-----

Build process is the same used in [Travis file](.travis.yml)
Reference build can be found [here](https://travis-ci.com/quicky2000/gif_player)

```
sudo apt-get update
sudo apt-get install -y libsdl1.2-dev
MY_LOCATION=`pwd`
mkdir ../repositories
cd ..
mv $MY_LOCATION repositories
QUICKY_REPOSITORY=`pwd`/repositories
export QUICKY_REPOSITORY
MY_LOCATION=`pwd`
cd $MY_LOCATION/repositories
git clone https://github.com/quicky2000/quicky_tools.git
git clone https://github.com/quicky2000/lib_gif.git
git clone https://github.com/quicky2000/parameter_manager.git
git clone https://github.com/quicky2000/quicky_exception.git
git clone https://github.com/quicky2000/simple_gui.git
git clone https://github.com/quicky2000/quicky_utils.git
cd quicky_tools/setup
. setup.sh
cd $MY_LOCATION
chmod a+x repositories/quicky_tools/bin/*
mkdir build
cd build
generate_makefile gif_player
make
```



