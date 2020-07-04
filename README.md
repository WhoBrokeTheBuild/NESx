# NESx

A Toy Nintendo Entertainment System Emulator

## Building

```sh
mkdir Build
cd Build
cmake ..

# Unix
make -l

# Any System
cmake --build .
```

## Build Flags

Build flags can be specified to cmake as `-DNAME=VALUE`, e.g.

```sh
cmake -DBUILD_TESTS=ON ..
```

* **BUILD_TOOLS**, default `OFF`

  Builds additional tools for debugging, assembling, etc.

* **BUILD_TESTS**, default `OFF`

  Builds automated tests.

* **BUILD_THIRDPARTY**, default `ON`

  Builds all missing required dependencies for the project.

## Running Tests

```sh
# Enable Tests
cmake -DBUILD_TESTS=ON ..

# Unix
make -l all test
# or
ctest --verbose

# Any System
cmake --build . --target test
```
