ROOT_FOLDER="$PWD"
SPDK_FOLDER="$PWD/spdk/src"
cd spdk/src
./configure
make

cp -r "$SPDK_FOLDER/include" "$ROOT_FOLDER/spdk/"

mkdir -p "$ROOT_FOLDER/spdk/lib/"

cd "$SPDK_FOLDER/build/lib"
for i in *.a; do ar -x "$i"; done
ar -qc libspdk.a *.o
mv libspdk.a "$ROOT_FOLDER/spdk/lib/"
echo "libspdk.a is generated." 

cd "$SPDK_FOLDER/dpdk/build/lib"
for i in *a; do ar -x "$i"; done
ar -qc librte.a *.o
mv librte.a "$ROOT_FOLDER/spdk/lib/"
echo "librte.a is generated."



