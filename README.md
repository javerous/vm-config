vm-config
===========

vm-config is a command line to show or change some VMware Fusion virtual machine configurations.


#### Example of commands to change configuration

- Enable SIP, like `csrutil enable`
  ```
  vm-config change my_vm.vmwarevm --csr-enable
  ```

- Disable SIP, like `csrutil disable`
  ```
  vm-config change my_vm.vmwarevm --csr-disable
  ```

- Disable SIP, with a specific macOS version flavor

  ```
  vm-config change my_vm.vmwarevm --csr-disable-version 11.0.0
  ```

- Change the UUID of the machine
  ```
  vm-config change my_vm.vmwarevm --machine-uuid 6E1881A7-41BF-4363-9419-6F0340DD6AE2
  ```

- Change the boot-args of the machine
  ```
  vm-config change my_vm.vmwarevm --boot-args 'debug=0x144'
  ```

- Change multiple configurations at the same time
  ```
  vm-config change my_vm.vmwarevm --csr-disable --boot-args 'amfi_get_out_of_my_way=0x1'
  ```


#### Example of commands to show configuration

- Show all (vmx content and nvram content)
  ```
  vm-config show my_vm.vmwarevm --all
  ```
