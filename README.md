# Readme
## Directory Structure
PA3 code base contains five directories, Makefile and other supported files required to complete the PA3 and described as follows:

**1.** The **include** directory contains a header file **crypter.h** for the crypter library. This header file provides function and data types declaration for the crypter library, and being used for the test-cases. Don't modify this header file.

**2.** The **src** directory contains skeleton code for the library you are going to implement for CryptoCard. This directory contains one file **crypter.c**. You need to complete the definition of the function provided in the **crypter.c**. After a successful build of the library, a shared library file **libcrypter.so** is being generated in the **lib** directory. Initially there is no such directory with name **lib** is present in the code. This directory will automatically be created by the **Makefile** on successful build of any test-cases.

**3.** The **test-cases** directory contains the test-cases that uses the library you are going to build for the CryptoCard.

## To Build System
Use **make** command to compile the assignment. After successful compilation, all the binaries for test-cases are generated in the root directory of the PA code base. Apart from the binary file, additionally two directory are generated namely, **lib** and **obj**. The directory **lib** holds the object code and shared library **libcrypter.so** for CryptoCard while **obj** populated with the object code of the test-cases.

**1. Compile code for all the test-cases:** To compile for all the test-cases, use the **make** command from the root directory of the PA code base.
> **make**

**2. Compile Individual test-case code:** Individual test-case code can be compiled using the **make** command followed by the test-case name in lower case. For example, if you want to build the code base for test-case 1, the syntax is as follows:
> **make test1**

Similarly, other test-cases can be compiled individually.

**3. Clean the build:**
> **make clean**

## Generated Binary Code
After the compilation of an individual test case code, the binary files are placed in the root directory of the PA code base.

Note: Add following line in your .bashrc file in your home directory(Eg: /home/user/.bashrc). Otherwise you will get error when you try to execute the test case binary file.
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<path to libcryptor.so library>

Eg:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/rohit/cs614/PA3/template/lib/



