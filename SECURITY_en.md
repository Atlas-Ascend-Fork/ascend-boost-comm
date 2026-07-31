# Security Statement

## System Security Hardening

- You can enable ASLR (level 2) during system runtime configuration to improve system security and enable system randomization protection.
Refer to the following method for configuration:

  ```
  echo 2 > /proc/sys/kernel/randomize_va_space
  ```

## Recommended Running Users

- To ensure security and minimize permissions, you are not advised to use administrator accounts such as `root`.

## File Permission Control

- You are advised to set the system `umask` value to `0027` or higher on hosts (both physical and virtual hosts) and containers. This ensures that new folders have a default maximum permission of `750` and new files have a default maximum permission of `640`.
- You are advised to take security measures such as permission control on sensitive content, including personal privacy data, business assets, source files, and various files saved during operator development. For example, permissions for the installation directory and files of the ascend-boost-comm repository must follow the recommendations in [Appendix A–Recommended Maximum Permissions for Files and Folders in Different Scenarios](#arecommended-maximum-permissions-for-files-and-folders-in-different-scenarios).
- During installation and usage, you must enforce proper permission control, referring to the same [Appendix A–Recommended Maximum Permissions for Files and Folders in Different Scenarios](#arecommended-maximum-permissions-for-files-and-folders-in-different-scenarios).

## Build Security Statement

- When you are building and installing ascend-boost-comm from the source code, some intermediate files will be generated. After the build is complete, you are advised to perform permission control on the intermediate files to ensure file security.

## Appendix

### A–Recommended Maximum Permissions for Files and Folders in Different Scenarios

| Type          | Maximum Linux Permission|
| -------------- | ---------------  |
| User's home directory                       |   750 (rwxr-x---)           |
| Program files (including scripts and library files)      |   550 (r-xr-x---)            |
| Program file directory                     |   550 (r-xr-x---)           |
| Configuration File                         |  640 (rw-r-----)            |
| Configuration file directory                     |   750 (rwxr-x---)           |
| Log files (recorded or archived)       |  440 (r--r-----)            |
| Log files (being recorded)               |    640 (rw-r-----)          |
| Log file directory                     |   750 (rwxr-x---)           |
| Debug files                        |  640 (rw-r-----)        |
| Debug file directory                    |   750 (rwxr-x---) |
| Temporary file directory                     |   750 (rwxr-x---)  |
| Maintenance and upgrade file directory                 |   770 (rwxrwx---)   |
| Service data files                     |   640 (rw-r-----)   |
| Service data file directory                 |   750 (rwxr-x---)     |
| Key components, private keys, certificates, and ciphertext file directory   |  700 (rwx------)     |
| Key components, private keys, certificates, and ciphertext files       | 600 (rw-------)     |
| APIs and scripts for encryption and decryption           |   500 (r-x------)       |
