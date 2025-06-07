rm -rf ./build
cmake . -B build
cd build
make
./bin/BTreeFy-PCExample
cd ..