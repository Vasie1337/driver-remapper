# Remap Driver

A shitty implementation of what i wanted to do. Should require some fixing to make it work.

## Components

The project consists of several key components:

### Driver

The core kernel-mode driver that handles memory operations, PML4 manipulation, and IOCTL communication.

### KDMapper

A driver mapper utility that facilitates loading unsigned drivers into the Windows kernel.

### Preloader

A component that prepares the system for driver loading and handles initialization.

### Usermode

The user-mode interface that communicates with the kernel driver to perform operations.

## Building

### Prerequisites

- Visual Studio 2019 or newer
- Windows Driver Kit (WDK)
- Windows SDK
- C++ development tools

### Compilation

1. Open `core.sln` in Visual Studio
2. Select the desired configuration (Debug/Release)
3. Build the solution

## Usage

1. Load the driver using the KDMapper utility
2. Use the usermode interface to communicate with the driver
3. Perform memory operations as needed

```cpp
// Example usage in usermode
if (!device_t.start_service()) {
    m_log("[-] Driver not loaded\n");
    return 0;
}

// Get process ID
device_t.m_pid = device_t.get_process_id("target.exe");

// Resolve DTB (Directory Table Base)
if (!device_t.resolve_dtb()) {
    m_log("[-] failed to get dtb\n");
    return FALSE;
}

// Get module base address
device_t.m_base = device_t.get_module_base(0);
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Disclaimer

This software is provided for educational and research purposes only. Usage of this software for attacking targets without prior mutual consent is illegal. The developers are not responsible for any misuse or damage caused by this program.

## Contributing

Contributions are welcome. Please feel free to submit a Pull Request.
