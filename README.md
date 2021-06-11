# NISystemConfigurationCLI

## Discovering a Real-Time System

### Option 1: List All Systems on Network

**Command:** `find`

**Description:** Finds all available hardware on the network and reports back the hostname, IP address, model, and serial number of each target.

**Example**

```
> nirtconfig find

Finding Available Targets...
HOSTNAME                           IP ADDR             MODEL          SERIAL NUMBER
NI-cDAQ-9134-01A044B2              10.1.128.222        cDAQ-9134      01A044B2
NI-cRIO-9030-01A0CF0D              10.1.128.52         cRIO-9030      01A0CF0D
NI-PXIe-8880-03182D37              10.1.128.113        PXIe-8880      03182D37
NI-PXIe8840-2F215A8E               10.1.128.48         PXIe-8840      2F215A8E
```
**Relevant Function Calls**
+ [NISysCfgFindSystems](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgfindsystems/)

**Implemented:** [nirtconfig_findAllTargets](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L112)

### Option 2: Find Target by Hostname or IP

**Command:** `find [TARGET_NAME]`

**Description:** Discover Real-Time system on the network from an IP address or hostname and reports back the hostname, IP address, model, and serial number.

**Example**

```
> nirtconfig find 10.1.128.113

HOSTNAME                           IP ADDR             MODEL          SERIAL NUMBER
NI-PXIe-8880-03182D37              10.1.128.113        PXIe-8880      03182D37
```

**Relevant Function Calls**
+ [NISysCfgInitializeSession](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgfindsystems/)

**Implemented:** [nirtconfig_findSingleTarget](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L93)

### Option 3: Find Target by Serial Number

**Command:** `findsn [SERIAL_NUMBER]`

**Description:** Discover Real-Time system on the network corresponding to some serial number. Call returns the IP address of that target.

**Example**

```
> nirtconfig findsn 03182D37

10.1.128.113
```

**Relevant Function Calls**
+ [NISysCfgFindSystems](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgfindsystems/)

**Implemented:** [nirtconfig_ipFromSerialNumber](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L473)

## Format a Target

**Command:** `format <TARGET_NAME>`

**Description:** Formats the system identified by the **TARGET_NAME** parameter.

**Example**
```
> nirtconfig format 10.1.128.42

Formatting...
```
**Relevant Function Calls**
+ [NISysCfgFormat](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgformat/)

**Implemented:** [nirtconfig_format](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L642)

## Set the Image of a Real-Time System

**Command:** `setimage [TARGET_NAME] [IMAGE_PATH]`

**Description:** Sets the system image located at **IMAGE_PATH** to designated **TARGET_NAME**.

**Example**
```
> nirtconfig setimage 10.1.128.131 "/home/mjacobson/Desktop/NI-PXIe-8861-BenJ"

Imaging Target: 10.1.128.131
Image Used: /home/mjacobson/Desktop/NI-PXIe-8861-BenJ
```

**Relevant Function Calls**
+ [NISysCfgSetSystemImageFromFolder2](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgsetsystemimagefromfolder2/)

**Implemented:** [nirtconfig_setImage](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L195)

## Get the Image of a Real-Time System

**Command:** `getimage [TARGET_NAME]`

**Description:** Gets the system image of **TARGET_NAME** and saves it to the present working directory. The hostname of the system will be used as the image folder's name.

**Example**
```
> nirtconfig getimage 10.1.128.131

Getting Image: 10.1.128.131
Saving To: "/home/mjacobson/Desktop/NI-PXIe-8861-BenJ"
```

**Relevant Function Calls**
+ [NISysCfgGetSystemImageAsFolder2](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfggetsystemimageasfolder2/)

**Implemented:** [nirtconfig_getImage](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L156)

## Update the Firmware of a System

**Command:** `updatefirmware [TARGET_NAME] [FIRMWARE_PATH] [-u USERNAME] [-p PASSWORD]`

**Description:** Updates firmware of **TARGET_NAME** with firmware located at **FIRMWARE_PATH**. Use the -u and -p flags to pass the username and password into the function call.

**Example**
```
> nirtconfig updatefirmware 10.1.128.42 "/home/mjacobson/Desktop/cRIO-9058_8.5.0.cfg" -u admin -p hunter2

Updating Firmware...
Target: 10.1.128.42
Firmware: /home/mjacobson/Desktop/SCM/NISystemConfigurationCLI/build/firmware/cRIO-9058_8.5.0.cfg
```

**Relevant Function Calls**
+ [NISysCfgUpgradeFirmwareFromFile](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgupgradefirmwarefromfile/)

**Implemented:** [nirtconfig_updateFirmware](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L392)

## Self Test Hardware

**Command:** `selftest [TARGET_NAME]`

**Description:** Self tests all hardware in **TARGET_NAME**. Prints the pass/fail results of every module.

**Example**
```
> nirtconfig selftest 10.1.128.42
Running Self Tests...
RESOURCE NAME                           PRODUCT NAME        PASS/FAIL      DETAILED RESULTS
cRIO1                                   NI cRIO-9058        Pass           
Mod1                                    NI 9871             Not Supported  
Mod2                                    NI 9237             Pass           
Mod3                                    NI 9205             Pass  
```

**Relevant Function Calls**
+ [NISysCfgSelfTestHardware](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgselftesthardware/)

**Implemented:** [nirtconfig_selfTest](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L221)

## Change the Hostname of a System

**Command:** `sethostname [TARGET_NAME] [NEW_TARGET_NAME]`

**Description:** Sets the hostname of **TARGET_NAME** to **NEW_TARGET_NAME**.

**Example**
```
> nirtconfig sethostname 10.1.128.131 Lab1-HIL1-8861

Updating Hostname of 10.1.128.131 to Lab1-HIL1-8861
```

**Relevant Function Calls**
+ [NISysCfgSetSystemProperty](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgsetsystemproperty/)
    + NISysCfgSystemPropertyHostname
+ [NISysCfgSaveSystemChanges](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgsavesystemchanges/)

**Implemented:** [nirtconfig_setHostname](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L297)

## Change the IP of a System

**Command:** `setip [TARGET_NAME] [NEW_IP_ADDRESS]`

**Description:** Sets the IP address of **TARGET_NAME** to **NEW_IP_ADDRESS**.

**Example**
```
> nirtconfig setip 10.1.128.42 10.1.128.43

Updating IP Address of 10.1.128.42 to 10.1.128.43
```

**Relevant Function Calls**
+ [NISysCfgSetSystemProperty](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgsetsystemproperty/)
    + NISysCfgSystemPropertyIpAddress
    + NISysCfgSystemPropertyIpAddressMode
+ [NISysCfgSaveSystemChanges](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgsavesystemchanges/)

**Implemented:** [nirtconfig_setIpAddress](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L329)

## Restart a System

**Command:** `restart [TARGET_NAME]`

**Description:** Restarts system identified by **TARGET_NAME**.

**Example**
```
> /nirtconfig restart 10.1.128.131

Restarting...
Restarted With IP Address: 10.1.128.131
```

**Relevant Function Calls**
+ [NISysCfgRestart](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgrestart/)

**Implemented:** [nirtconfig_restartTarget](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L362)

## Change the Programming Mode of a C-Series Module

**Command:** `setmode [TARGET_NAME] [scan|fpga|daq]`

**Description:** Sets the programming mode of every module in **TARGET_NAME** to scan engine (scan), real-time (daq), or FPGA (fpga).

**Example**
```
> nirtconfig setmode 10.1.128.42 fpga

Setting Module Mode: Mod1 (NI 9871)
Setting Module Mode: Mod2 (NI 9237)
Setting Module Mode: Mod3 (NI 9205)
```
**Relevant Function Calls**
+ [NISysCfgSetSystemProperty](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgsetsystemproperty/)
    + NISysCfgResourcePropertyModuleProgramMode
+ [NISysCfgSaveSystemChanges](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgsavesystemchanges/)

**Implemented:** [nirtconfig_setModuleMode](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L513)

## Change the Alias of a Module

**Command:** `setalias <TARGETNAME> <SLOT> <NEW_ALIAS>`

**Description:** Changes the alias of the resouce in slot number **SLOT** to **NEW_ALIAS**.

**Example**
```
> nirtconfig setalias 10.1.128.131 5 serialModule
```
**Relevant Function Calls**
+ [NISysCfgRenameResource](https://zone.ni.com/reference/en-XX/help/373242N-01/nisyscfgcvi/nisyscfgrenameresource/)

**Implemented:** [nirtconfig_setAlias](https://github.com/jacobson3/NISystemConfigurationCLI/blob/main/src/nirtconfig.c#L671)
