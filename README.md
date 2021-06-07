# NISystemConfigurationCLI

### **find**

Finds all available hardware on the network and reports back the hostname, IP address, model, and serial number of each target.

### **find [TARGET_NAME]**

Finds the hardware given by the **TARGET_NAME** and reports back the hostname, IP address, model, and serial number.

### **setimage [TARGET_NAME] [IMAGE_PATH]**

Sets the system image located at **IMAGE_PATH** to **TARGET_NAME**

### **getimage [TARGET_NAME]**

Gets the system image of **TARGET_NAME** and saves it to the present working directory. The hostname of the system will be used as the image folder's name.

### **selftest [TARGET_NAME]**

Self tests all hardware in **TARGET_NAME**. Prints the pass/fail results of every module.

### **sethostname [TARGET_NAME] [NEW_TARGET_NAME]**

Sets the hostname of **TARGET_NAME** to **NEW_TARGET_NAME**.

### **setip [TARGET_NAME] [NEW_IP_ADDRESS]**

Sets the IP address of **TARGET_NAME** to **NEW_IP_ADDRESS**.

### **restart [TARGET_NAME]**

Restarts system identified by **TARGET_NAME**.

### **updatefirmware [TARGET_NAME] [FIRMWARE_PATH] [-u USERNAME] [-p PASSWORD]**

Updates firmware of **TARGET_NAME** with firmware located at **FIRMWARE_PATH**. Use the -u and -p flags to pass the username and password into the function call.

### **findsn [SERIAL_NUMBER]**

Searches network for system with **SERIAL_NUMBER** and returns the IP address.

### **setmode [TARGET_NAME] [scan|fpga|daq]**

Sets the programming mode of every module in **TARGET_NAME** to scan engine, real-time (daq), or FPGA.