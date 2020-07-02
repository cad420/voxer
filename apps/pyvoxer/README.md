# pyvoxer

Python binding for voxer

## Compilation
You can provide a `PYTHON_EXECUTABLE` cmake variable to overwrite the default python path.

Compile under voxer project
```shell script
cd voxer
mkdir build && cd build
cmake ..
cmake --build . --target pyvoxer
```

Standalone compilation (with installed voxer) 
```shell script
mkdir build && cd build
cmake ..
cmake --build .
```
## Instalation

Development installation:

```shell script
cd pyvoxer
pip install -e .
```