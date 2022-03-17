# Multiprocessor Programming Project

#### Initial steps I needed to get it working

```
- Make a remote SSH server on Intel CPU + iGPU running Debian
- Install cmake, g++, cc, and rsync on Server
- Install CLion on Laptop
- Setup remote development
  - https://www.jetbrains.com/help/clion/remote-projects-support.html
- Install clinfo, beignet-opencl-icd, ocl-idc-opencl-dev on Server
- Get an OpenCL example
  - https://raw.githubusercontent.com/KhronosGroup/OpenCL-SDK/main/samples/core/enumopencl/main.cpp
- Disable CLion Tar/GZip
  - Copies symbolic links
  - https://stackoverflow.com/a/67802186
- usermod -a -G render %user%
```

#### Repo history contain other weeks