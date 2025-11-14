# GWTONN Program Compiler

## About the Project

A compiler to compile (dutch) program language. The output will be an object file that can be loaded on the STM32 as a **program** in the specified space (see Firmware for more info). 

## Lanuage specification

The compiler compiles a customized language. The specifications of the language can be found [here](./gwtonn_language_specificaton.md)

## Prerequisites

| Tool      | Minimum version | How to install |
| :--       | :-- | :--|
| CMake     | 	3.22 |	apt-get install cmake / brew install cmake / choco install cmake |
| Compiler	| C11‑compatible (GCC ≥ 7, Clang ≥ 5, MSVC ≥ 19.15)	| - |

## Getting Started

```sh
git clone [https://github.com/Ge-Wit-t-Oit-Noit-Nie/gpc.git](https://github.com/Ge-Wit-t-Oit-Noit-Nie/gpc.git)
cd gpc
git submodule update --init   # if you use submodules
```

### build & Install

```sh
# 1️⃣ Create a build directory (keeps source clean)
mkdir -p build && cd build

# 2️⃣ Configure the project
cmake .. \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \

# 3️⃣ Compile
cmake --build . --parallel $(nproc)

# 5️⃣ Install (optional)
sudo cmake --install .
```

## Usage

In order to use the compiler, you can execute the following command

```sh
GPC 
```

## Contributing

Please add to the repository if something is wrong or can be updated.

1. Fork the repo
2. Create a feature branch (git checkout -b feat/awesome-feature)
3. Write tests for your change
4. Submit a Pull Request (PR)
5. PR must pass CI and adhere to the coding style (clang-format -i or cmake -DSTYLE_CHECK=ON ..)
6. You can also link to a more detailed CONTRIBUTING.md file if you have one.

## License

This project is licensed under the MIT License – see the [LICENSE](./LICENSE.md) file for details.
