
# ZDK (Zen Development Kit)

**ZDK** is a curated collection of zero-dependency, header-only C libraries for robust system and game development. It aggregates the `z-libs` suite into a single, installable package.

It is designed to provide a "Standard Library Extension" experience, offering essential data structures (vectors, maps, lists), math utilities, error handling, ... without the bloat of large frameworks.

## Contents

| Library | Description | Included in `zworld.h`? |
| :--- | :--- | :---: |
| **zvec.h**    | Type-safe generic vector (dynamic array). | Yes |
| **zmap.h**    | Type-safe generic hash map. | Yes |
| **ztree.h**   | Type-safe generic red-black tree. | Yes |
| **zlist.h**   | Type-safe generic doubly linked list. | Yes |
| **zmath.h**   | Fast, software-based math library (vectors, matrices, trig). | Yes |
| **zstr.h**    | Modern string library with small string optimization (SSO) and Views. | Yes |
| **zrand.h**   | Pseudo-random number generation utilities. | Yes |
| **zthread.h** | Type-safe, cross-platform threading and synchronization library. | Yes |
| **zfile.h**   | Cross-platform file system library.  | Yes |
| **zwasm.h**   | Freestanding WebAssembly binary generator. | Yes |
| **znet.h**    | Networking library that unifies TCP, UDP, and HTTP. | Yes |
| **ztime.h**   | simple, cross-platform time and clock library. | Yes |
| **zerror.h**  | Result types (`Result<T,E>`) and stack tracing. | No |
| **zalloc.h**  | Custom memory allocator wrappers and tracking. | No |
| **zops.h**    | Polymorphic API that glues the rest of `z-libs` | No |

## Installation

### Debian / Ubuntu (.deb)
Go to the [Releases](https://github.com/z-libs/zdk/releases) page (or the latest Action run), download the `.deb` file, and run 

```bash
sudo apt install ./zdk_latest_all.deb`.
```

> Installs headers to `/usr/include/zdk/`.

### Clib
If you use the [clib](https://github.com/clibs/clib) package manager, run:

```bash
clib install z-libs/zdk`.
```

### Manual Install

```bash
git clone https://github.com/z-libs/zdk.git
cd zdk
sudo make install
```

## Usage

### The Core World
Include `<zdk/zworld.h>` to get the standard data structures and math utilities immediately. This makes tools like `zvec`, `zlist`, `zmap`, `zstr`, `zrand`, and `zmath` available for use in your `main` function.

```c
#define ZMATH_IMPLEMENTATION
#include <zdk/zworld.h> 
DEFINE_VEC_TYPE(int, Int)

int main(void) 
{
    // zvec, zmap, zstr, zmath, ... are all available here.
    vec_autofree(Int) numbers = vec_init(Int);
    vec_push(&numbers, 100);
    
    float x = zmath_sin(1.5f);
    
    return 0;
}

```

> Note: don't forget using the scanner or making your own tables for the generic containers.

### Infrastructure (Allocators, Errors, Ops)
System-level libraries (`zerror`, `zalloc`, `zops`) are **not** included in `zworld.h` by default. This is because they often require configuration macros (like defining custom mallocs or implementation flags) before inclusion.

```c
#define ZERROR_IMPLEMENTATION // Define implementation in one file.
#include <zdk/zerror.h>
#include <zdk/zworld.h>

DEFINE_VEC_TYPE(int, Int)

int main(void) 
{
    // Use zerror features.
    zres result = zres_ok();
    
    // Use zworld features.
    vec_autofree(Int) v = vec_init(Int);
    
    return 0;
}
```

You must include them manually. For example, to use `zerror`, you would define `ZERROR_IMPLEMENTATION` before including `<zdk/zerror.h>`, and then include `<zdk/zworld.h>` for the rest of the suite.

## Updates

This repository is **automatically synced** with the individual upstream repositories every 12 hours. 
To contribute to a specific module, please visit its respective repository at [github.com/z-libs](https://github.com/z-libs).
