# Eunomia Build Guide

This guide helps you compile the Eunomia project from a fresh git clone.

## Prerequisites

### System Dependencies

Install the required system packages:

```bash
sudo apt-get update
sudo apt-get install -y libelf-dev clang llvm libcurl4-openssl-dev
```

### Initialize Git Submodules

**IMPORTANT**: This project uses git submodules. After cloning, you must initialize them:

```bash
git submodule update --init --recursive
```

This will fetch:
- `libbpf` - The BPF library
- `third_party/prometheus-cpp` - Prometheus C++ client library

## Building

### Option 1: Using Makefile (Recommended)

The Makefile provides convenient targets:

```bash
# Install all dependencies (requires sudo)
make install-deps

# Generate BPF tools
make generate-tools

# Build the project
make install
```

### Option 2: Using CMake Directly

```bash
# Generate BPF tools first
make generate-tools

# Configure and build
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Common Build Issues

### Issue: "No such file or directory" for libbpf

**Cause**: Git submodules not initialized  
**Solution**: Run `git submodule update --init --recursive`

### Issue: "gelf.h: No such file or directory"

**Cause**: Missing libelf-dev  
**Solution**: Run `sudo apt-get install libelf-dev`

### Issue: "llvm-strip: not found"

**Cause**: Missing LLVM tools  
**Solution**: Run `sudo apt-get install llvm`

### Issue: "Could NOT find CURL"

**Cause**: Missing libcurl development files  
**Solution**: Run `sudo apt-get install libcurl4-openssl-dev`

## Testing

Run tests with:

```bash
make test
```

## Documentation

Generate documentation with:

```bash
make docs
```

## Note on Header Files

All required header files are included in the repository:
- Core headers: `include/eunomia/`
- Helper headers: `include/helpers/`
- Tracker headers: `bpftools/*/`

If you encounter "missing header" errors, it's likely due to:
1. Uninitialized git submodules (most common)
2. Missing system dependencies
3. Incomplete build of BPF tools (`make generate-tools` not run)

The issue is NOT missing committed header files - all are present in the repository.
