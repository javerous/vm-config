vm-config
===========

vm-config is a command line to show or change some VMware Fusion virtual machine configurations.

## Build

- **Build on macOS only (Xcode 14 minimum)**
  - Open `vm-config.xcodeproj` in Xcode and simply build `vm-config` target.
  - Execute `xcodebuild archive -project 'vm-config.xcodeproj' -scheme 'vm-config' -configuration 'Release' -archivePath 'vm-config.xcarchive'`

- **Build on macOS or Linux**
  ```
  $ cd vm-config
  $ mkdir build
  $ cd build
  $ cmake ../
  $ make
  ```
  
  On Linux, you may need to install `libbsd` and `libuuid` before. For example:
  ```
  sudo apt install libbsd-dev
  sudo apt install uuid-dev
  ```


## Usage

#### Change virtual machine configuration

- Enable SIP, like `csrutil enable`
  ```
  vm-config change my_vm.vmwarevm --csr-enable
  ```

- Disable SIP, like `csrutil disable`
  ```
  vm-config change my_vm.vmwarevm --csr-disable
  ```

- Disable SIP, like `csrutil disable`, but with a specific macOS version flavor

  ```
  vm-config change my_vm.vmwarevm --csr-disable-version 11.0.0
  ```

- Change the UUID of the machine
  ```
  vm-config change my_vm.vmwarevm --machine-uuid 6E1881A7-41BF-4363-9419-6F0340DD6AE2
  vm-config change my_vm.vmwarevm --machine-uuid `uuidgen`
  ```

- Change the boot-args of the machine
  ```
  vm-config change my_vm.vmwarevm --boot-args 'debug=0x144'
  ```
  
- Change the screen resolution of the machine
  ```
  vm-config change my_vm.vmwarevm --screen-resolution 1024x768
  ```

- Change multiple configurations at the same time
  ```
  vm-config change my_vm.vmwarevm --csr-disable --boot-args 'amfi_get_out_of_my_way=0x1'
  ```


#### Show virtual machine configuration

- Show all (vmx content and nvram content)
  ```
  vm-config show my_vm.vmwarevm --all
  ```
  
- Show only vmx content
  ```
  vm-config show my_vm.vmwarevm --vmx
  ```

- Show only nvram content
  ```
  vm-config show my_vm.vmwarevm --nvram
  ```

- Show only nvram EFI variable entries
  ```
  vm-config show my_vm.vmwarevm --nvram-efi-variables
  ```

- Show only a specific nvram EFI variable entry
  ```
  vm-config show my_vm.vmwarevm --nvram-efi-variable csr-active-config
  ```
