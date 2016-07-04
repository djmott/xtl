eXtended Template Library
=========================
[![Open Hub project report](https://www.openhub.net/p/libxtl/widgets/project_thin_badge.gif)](https://www.openhub.net/p/libxtl)
[![Travis](https://img.shields.io/travis/djmott/xtl.svg?style=plastic)](https://travis-ci.org/djmott/xtl)
[![Coveralls branch](https://img.shields.io/coveralls/djmott/xtl.svg?style=plastic)](https://coveralls.io/github/djmott/xtl)
[![SonarQube Tech Debt](https://img.shields.io/sonar/https/sonarqube.com/xtl/tech_debt.svg)](https://sonarqube.com/overview?id=xtl)
[![SonarQube Quality Gate](http://nemo.sonarqube.org/api/badges/gate?key=xtl&blinking=true)](https://sonarqube.com/overview?id=xtl)
[![Boost License](https://img.shields.io/badge/license-Boost_Version_1.0-green.svg?style=plastic)](http://www.boost.org/LICENSE_1_0.txt)

XTL is a series of C++ template metaprogramming patterns, idioms, algorithms and libraries that solve a variety of programming tasks. It supplements, extends and cooperates with the STL by providing some frequently used components that are otherwise absent from the standard.

### Getting started

XTL works with modern C++11 compilers and has been tested with MinGW, GCC, Intel C++, Cygwin and Microsoft Visual C++. The library can be used out-of-the-box in many cases by simply including the desired header since most components are header-only. A few components require linking to a run-time component so they will need to be compiled.

### Requirements

* [CMake](http://www.cmake.org) is required to configure
* [libiconv](https://www.gnu.org/software/libiconv/) is optional for unicode support on Posix platforms.
* [libuuid](https://sourceforge.net/projects/libuuid/) is optional for UUID/GUID support on Posix plaforms. (This library has bounced around to several locations over the years. Some documentation says it's included in modern Linux kernel code while others say it's included in the e2fsprogs package. Most modern Linux distros support some version in their respective package managers.)

### Obtaining

XTL is hosted on GitHub and is available at http://www.github.io/djmott/xtl
Checkout the repo with git:

```
git clone https://github.com/djmott/xtl.git
```

### Compiling

For the most part XTL is a 'header-only' library so compilation isn't necessary. None the less, it must be configured for use with the compiler and operating system with [CMake](https://cmake.org/). From within the top level directory:

```
mkdir build
cd build
cmake ..
```
The compilation step is not always necessary depending on the required components that will be used. The method used to compile the run-time code is platform, toolchain and CMake configuration specific. For Linux, Cygwin and MinGW make files just run `make`.
### Using
Several configuration options are available during configuration with CMake. For most purposes the default configuration should work fine. Applications should add the `include` folder to the search path. The configuration with CMake detects the compiler toolchain and target operating system then produces the primary include file. For most applications just including the project header will go a long way:
```{.cpp}
 #include <xtd/xtd.hpp>
```

### Testing

XTL uses the [Google Test](https://github.com/google/googletest) framework for unit tests and system test. From within the build directory:
```
make unit_tests
```
The unit tests and system tests are contained in the same resulting binary at `tests/unit_tests`. The `coverage_tests` build target is only available for GCC:
```
make coverage_tests
```
This will produce the binary `tests/coverage_tests` which is identical to the `tests/unit_tests` binary but has additional instrumenting enabled for gcov.

### Documentation

[Doxygen](http://www.doxygen.org) is used to generate source documentation. The code is fairly well marked up for doxygen generation. After the project has been configured with CMake build with documentation with:

```
make docs
```
This will extract the source comments and generate nice documentation in the `docs/html` folder. Also available is the [wiki](https://github.com/djmott/xtl/wiki)

### Feedback and Issues

Submit a [ticket](https://github.com/djmott/xtl/issues) on GitHub if a bug is found. Effort will be made to fix it ASAP.

### Contributing

Contributions are appreciated. To contirube, <a class="github-button" href="https://github.com/djmott/xtl/fork" data-icon="octicon-repo-forked" data-style="mega" data-count-href="/djmott/xtl/network" data-count-api="/repos/djmott/xtl#forks_count" data-count-aria-label="# forks on GitHub" aria-label="Fork djmott/xtl on GitHub">fork</a>
the project, add some code and submit a [pull request](https://github.com/djmott/xtl/pulls). In general, contributions should:
* Clear around %80 in code coverage tests
* Pass SonarQube quality gateway
* Pass unit and system tests
* Pass tests through ValGrind memcheck or some other dynamic analysis with no resource leaks or other significant issues

### License

XTL is copyright by David Mott and licensed under the Boost Version 1.0 license agreement. See [LICENSE.md](LICENSE.md) or [http://www.boost.org/LICENSE_1_0.txt](http://www.boost.org/LICENSE_1_0.txt) for license details. 
