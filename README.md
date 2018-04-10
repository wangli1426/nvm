## Pull-Mode Asynchronous B+-Tree
This is a pull-mode asynchronous B+-tree (PA-tree) implementation, tailored to the performance characteristics of the NVM Express interface.

### 1. Compile

1. Install spdk drive

```
bash configure_spdk.sh
```

2. Compile the source code

```
mkdir build
cd build
cmake ..
make
```

### 2. Run the benchmark

1. Mount nvme device
```
sudo ../src/spdk/scripts/setup.sh
```

2. Run the benchmark
```
sudo ./tree_benchmark
```

Note: sudo previlage is required to allocate NVMe namespace.
