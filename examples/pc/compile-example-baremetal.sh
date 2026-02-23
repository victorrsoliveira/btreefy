rm -rf ./build_baremetal
cmake . -B build_baremetal -DBTREEFY_RUNNER=BAREMETAL
cd build_baremetal
make
./bin/BTreeFy-PCExample
cd ..