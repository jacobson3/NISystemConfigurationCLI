#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <nisyscfg/nisyscfg.h>
#include "nirtconfig.h"

static const struct
{
    const char* const name;
    int (*fpointer)(int argc, char** argv);
} nirtFunctions[] =
{
    {"find", nirtconfig_find},
    {"setimage", nirtconfig_setImage},
    {"getimage", nirtconfig_getImage},
    {"selftest", nirtconfig_selfTest},
    {"sethostname", nirtconfig_setHostname},
    {"setip", nirtconfig_setIpAddress},
    {"restart", nirtconfig_restartTarget},
    {"updatefirmware", nirtconfig_updateFirmware},
    {"findsn", nirtconfig_ipFromSerialNumber},
    {"setmode", nirtconfig_setModuleMode},
    {NULL, NULL}
};


int main(int argc, char** argv)
{
    int status = 0;

    if (argc > 1) //Command passed as argument
    {
        std::string command = argv[1];
        int i = 0;

        while (nirtFunctions[i].name != NULL) //Iterate through function lookup table
        {
            if (nirtFunctions[i].name == command) //Check if command matches function in table
            {
                status = (*nirtFunctions[i].fpointer)(argc, argv);
                break;
            }
            i++;
        }

        if (nirtFunctions[i].name == NULL) printf("Invalid Command: %s\n", argv[1]);
    }

    else //No arguments passed
    {
        printf("No Function Passed\n");
        status = 1;
    }

    if (status != 0) printf("Error: %d\n", status);
    return status;
}

int nirtconfig_find(int argc, char** argv)
{
    int status = 0;

    if (argc > 2) //IP address passed as argument, find specific target
    {
        status = nirtconfig_findSingleTarget(argv[2]);

        return status;
    }

    else //no arguments passed, find all available targets
    {
        status = nirtconfig_findAllTargets();
    }    

    return status;
}

int nirtconfig_findSingleTarget(char *targetName)
{
    int status = 0;
    NISysCfgSessionHandle session = NULL;

    status = NISysCfgInitializeSession(targetName, NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0) //Error initializeing session
    {
        printf("Target Not Found\n");
        return status;
    }
    
    printf("%-35s%-20s%-15s%s\n", "HOSTNAME", "IP ADDR", "MODEL", "SERIAL NUMBER");
    nirtconfig_printSystemInfo(session);

    status = NISysCfgCloseHandle(session);

    return status;
}

int nirtconfig_findAllTargets()
{
    NISysCfgEnumSystemHandle enumSystemHandle = NULL;
    NISysCfgSessionHandle session = NULL;
    char systemName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    int status = 0;
    
    printf("Finding Available Targets...\n");

    status = NISysCfgFindSystems(NULL, NULL, NISysCfgBoolTrue, 
                            NISysCfgIncludeCachedResultsOnlyIfOnline, NISysCfgSystemNameFormatHostname,
                            10000,NISysCfgBoolTrue,&enumSystemHandle);

    printf("%-35s%-20s%-15s%s\n", "HOSTNAME", "IP ADDR", "MODEL", "SERIAL NUMBER");

    while(status = NISysCfgNextSystemInfo(enumSystemHandle, systemName) == NISysCfg_OK) //Iterate through systems found
    {
        NISysCfgInitializeSession(systemName, NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

        nirtconfig_printSystemInfo(session);
        status = NISysCfgCloseHandle(session);
    }

    status = NISysCfgCloseHandle(enumSystemHandle);

    return status;
}

void nirtconfig_printSystemInfo(NISysCfgSessionHandle session)
{
    char model[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char serialNumber[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char hostname[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char ipaddr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";

    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertyHostname, hostname);
    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertyProductName, model);
    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertyIpAddress, ipaddr);
    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertySerialNumber, serialNumber);

    printf("%-35s%-20s%-15s%s\n", hostname, ipaddr, model, serialNumber);
}

int nirtconfig_getImage(int argc, char** argv)
{
    int status = 0;

    if (argc > 2) //argument passed
    {
        NISysCfgSessionHandle session = NULL;

        status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault, 
                                            NISysCfgBoolFalse, 10000, NULL, &session);

        if (status != 0) //error opening session
        {
            printf("Unable to Connect to Target\n");
            return status;
        }

        printf("Getting Image: %s\n", argv[2]);

        char destination[256] = "";
        nirtconfig_buildOutputDir(session, destination);

        printf("Saving To: \"%s\"\n", destination);

        status = NISysCfgGetSystemImageAsFolder2(session, NISysCfgBoolTrue, destination,
                                         NULL, 0, NULL, NISysCfgBoolTrue, NISysCfgBoolFalse);
    }

    else //no arguments passed
    {
        printf("No Target Selected\n");
        status = 1;
    }

    return status;
}

void nirtconfig_buildOutputDir(NISysCfgSessionHandle session, char *pathBuffer)
{
    char hostname[NISYSCFG_SIMPLE_STRING_LENGTH] = "";

    getcwd(pathBuffer, 256);
    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertyHostname, hostname);

    strcat(pathBuffer, "/");
    strcat(pathBuffer, hostname);
}

int nirtconfig_setImage(int argc, char **argv)
{
    if (argc != 4) //Check for correct number of arguments
    {
        printf("Error Expecting Arguments: setimage <TARGETNAME> <IMAGEPATH>\n");
        return 0;
    }

    int status = 0;
    NISysCfgSessionHandle session = NULL;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0) //error opening session
    {
        printf("Unable to Connect to Target: %s\n", argv[2]);
        return status;
    }

    printf("Imaging Target: %s\nImage Used: %s\n", argv[2], argv[3]);

    status = NISysCfgSetSystemImageFromFolder2(session, NISysCfgBoolTrue, argv[3], "", 0, NULL, 
                                            NISysCfgBoolFalse, NISysCfgPreservePrimaryResetOthers);
    
    NISysCfgCloseHandle(session);
    return status;
}

int nirtconfig_selfTest(int argc, char** argv)
{
    if (argc != 3) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: selftest <TARGETNAME>");
        return 0;
    }

    NISysCfgSessionHandle session = NULL;
    int status = 0;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0) //error opening session
    {
        printf("Unable to Connect to Target: %s\n", argv[2]);
        return status;
    }

    NISysCfgEnumResourceHandle resourceHandle = NULL;
    NISysCfgResourceHandle resource = NULL;
    NISysCfgFilterHandle filter = NULL;

    printf("Running Self Tests...\n");
    
    NISysCfgCreateFilter(session, &filter);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertySlotNumber, 0);

    NISysCfgFindHardware(session, NISysCfgFilterModeAllPropertiesExist, filter, NULL, &resourceHandle);

    printf("%-40s%-20s%-15s%s\n", "RESOURCE NAME", "PRODUCT NAME", "PASS/FAIL", "DETAILED RESULTS");
    while (status = NISysCfgNextResource(session, resourceHandle, &resource) == NISysCfg_OK) //Iterate through all hardware resources
    {
        nirtconfig_printSelfTestResults(resource);
        NISysCfgCloseHandle(resource);
    }

    status = NISysCfgCloseHandle(resourceHandle);
    status = NISysCfgCloseHandle(session);

    return status;
}

void nirtconfig_printSelfTestResults(NISysCfgResourceHandle resource)
{
    char productName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char resourceName [NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char alias [NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char passFail [NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char* detailedResults = NULL;
    int status = 0;

    NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyProductName, productName);
    NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertResourceName, 0, resourceName);
    NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertUserAlias, 0, alias);

    status = NISysCfgSelfTestHardware(resource, 0, &detailedResults);

    switch (status) //populate pass/fail string
    {
        case NISysCfg_OK:
            strcpy(passFail, "Pass");
            break;
        case NISysCfg_NotImplemented:
            strcpy(passFail, "Not Supported");
            break;
        default:
            sprintf(passFail, "Error: %d", status);
    }

    //print results using resource's alias if available
    if (strlen(alias)) printf("%-40s%-20s%-15s%s\n", alias, productName, passFail, detailedResults);
    else printf("%-40s%-20s%-15s%s\n", resourceName, productName, passFail, detailedResults);

    NISysCfgFreeDetailedString(detailedResults);

}

int nirtconfig_setHostname(int argc, char** argv)
{
    if (argc != 4) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: sethostname <TARGETNAME> <NEW_TARGETNAME>\n");
        return 0;
    }

    NISysCfgSessionHandle session = NULL;
    int status = 0;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0) //error opening session
    {
        printf("Unable to Connect to Target: %s\n", argv[2]);
        return status;
    }

    printf("Updating Hostname of %s to %s\n", argv[2], argv[3]);

    status = NISysCfgSetSystemProperty(session, NISysCfgSystemPropertyHostname, argv[3]);

    NISysCfgBool restartRequired;
    char* detailedResults = NULL;

    NISysCfgSaveSystemChanges(session, &restartRequired, &detailedResults);
 
    NISysCfgFreeDetailedString(detailedResults);
    NISysCfgCloseHandle(session);

    return status;
}

int nirtconfig_setIpAddress(int argc, char** argv)
{
    if (argc < 4) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: sethostname <TARGETNAME> <NEW_IP>\n");
        return 0;
    }

    NISysCfgSessionHandle session = NULL;
    int status = 0;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0) //error opening session
    {
        printf("Unable to Connect to Target: %s\n", argv[2]);
        return status;
    }

    printf("Updating IP Address of %s to %s\n", argv[2], argv[3]);

    status = NISysCfgSetSystemProperty(session, NISysCfgSystemPropertyIpAddress, argv[3]);
    status = NISysCfgSetSystemProperty(session, NISysCfgSystemPropertyIpAddressMode, NISysCfgIpAddressModeStatic);

    NISysCfgBool restartRequired;
    char* detailedResults = NULL;

    status = NISysCfgSaveSystemChanges(session, &restartRequired, &detailedResults);
    
    NISysCfgFreeDetailedString(detailedResults);
    NISysCfgCloseHandle(session);

    return status;
}

int nirtconfig_restartTarget(int argc, char** argv)
{
    if (argc != 3) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: restart <TARGETNAME>\n");
        return 0;
    }

    NISysCfgSessionHandle session = NULL;
    int status = 0;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0) //error opening session
    {
        printf("Unable to Connect to Target: %s\n", argv[2]);
        return status;
    }

    char ipAddr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";

    printf("Restarting...\n");
    status = NISysCfgRestart(session, NISysCfgBoolTrue, NISysCfgBoolFalse, NISysCfgBoolFalse, 120000, ipAddr);
    if (status == 0) printf("Restarted With IP Address: %s\n", ipAddr);
    NISysCfgCloseHandle(session);

    return status;
}

int nirtconfig_updateFirmware(int argc, char** argv)
{
    if (argc < 4) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: updatefirmware <TARGETNAME> <FIRMWARE_PATH>\n");
        return 0;
    }

    NISysCfgSessionHandle session = NULL;
    char username[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char password[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    int status = 0;

    nirtconfig_getCredentials(argc, argv, username, password);

    status = NISysCfgInitializeSession(argv[argc-2], username, password, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0) //error opening session
    {
        printf("Unable to Connect to Target: %s\n", argv[argc-2]);
        return status;
    }

    NISysCfgResourceHandle resource = NULL;
    NISysCfgFirmwareStatus firmwareStatus;
    char* detailedResults = NULL;
    int percentComplete = 0;

    if (status = status = nirtconfig_findFirmwareResource(session, &resource) == NISysCfg_OK) //Get hardware resource
    {
        status = NISysCfgUpgradeFirmwareFromFile(resource, argv[argc-1], NISysCfgBoolTrue, NISysCfgBoolTrue, NISysCfgBoolFalse, &firmwareStatus, &detailedResults);

        if (status == NISysCfg_OK) printf("Updating Firmware...\nTarget: %s\nFirmware: %s\n", argv[argc-2], argv[argc-1]);
        else return status;

        while (firmwareStatus < 0) //While firmware is being updated
        {
            NISysCfgCheckFirmwareStatus(resource, &percentComplete, &firmwareStatus, &detailedResults);
            sleep(1);
        }

        printf("Firmware Status: %d\nDetailed Results: %s\n", firmwareStatus, detailedResults);

    }

    NISysCfgFreeDetailedString(detailedResults);
    NISysCfgCloseHandle(resource);
    NISysCfgCloseHandle(session);

    return status;
}

void nirtconfig_getCredentials(int argc, char** argv, char* username, char* password)
{
    int flag;

    while ((flag = getopt(argc, argv, "u:p:")) != -1)
    {
        switch (flag)
        {
            case 'u':
                strcpy(username, optarg);
                break;

            case 'p':
                strcpy(password, optarg);
                break;
            
            default: break;
        }
    }
}

int nirtconfig_findFirmwareResource(NISysCfgSessionHandle session, NISysCfgResourceHandle *resource)
{
    int status = 0;
    NISysCfgEnumResourceHandle resourceHandle = NULL;
    NISysCfgFilterHandle filter = NULL;
 
    //Set up filter
    NISysCfgCreateFilter(session, &filter);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertySupportsFirmwareUpdate, NISysCfgBoolTrue);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyResourceName, "system");

    //Find resource 
    NISysCfgFindHardware(session, NISysCfgFilterModeMatchValuesAll, filter, NULL, &resourceHandle);
    status = NISysCfgNextResource(session, resourceHandle, resource);

    //Free memory
    NISysCfgCloseHandle(resourceHandle);
    
    return status;
}

int nirtconfig_ipFromSerialNumber(int argc, char** argv)
{
    if (argc < 3) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: findsn <SERIAL_NUMBER> \n");
        return 0;
    }
    NISysCfgEnumSystemHandle enumSystemHandle = NULL;
    NISysCfgSessionHandle session = NULL;
    char systemIP[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char serialNumber[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    int status = 0;
    
    status = NISysCfgFindSystems(NULL, NULL, NISysCfgBoolTrue, 
                            NISysCfgIncludeCachedResultsOnlyIfOnline, NISysCfgSystemNameFormatIp,
                            10000,NISysCfgBoolTrue,&enumSystemHandle);

    while(status = NISysCfgNextSystemInfo(enumSystemHandle, systemIP) == NISysCfg_OK) //Iterate through systems found
    {
        NISysCfgInitializeSession(systemIP, NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

        NISysCfgGetSystemProperty(session, NISysCfgSystemPropertySerialNumber, serialNumber);
        NISysCfgCloseHandle(session);

        if( (status = strcmp(argv[2], serialNumber)) == 0) //Found correct target
        {
            NISysCfgCloseHandle(enumSystemHandle);
            printf("%s\n", systemIP);
            return 0;
        }
    }

    printf("Target With SN %s Not Found\n", argv[2]);
    NISysCfgCloseHandle(enumSystemHandle);

    return 1;
}

int nirtconfig_setModuleMode(int argc, char** argv)
{
    if (argc < 4) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: setmode <TARGETNAME> <scan|fpga|daq>\n");
        return 1;
    }

    NISysCfgSessionHandle session = NULL;
    int status = 0;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0) //error opening session
    {
        printf("Unable to Connect to Target: %s\n", argv[argc-2]);
        return status;
    }

    //Get module programming mode
    NISysCfgModuleProgramMode moduleMode = NISysCfgModuleProgramModeNone;
    if (strcmp(argv[3], "scan") == 0) moduleMode = NISysCfgModuleProgramModeRealtimeScan;
    else if (strcmp(argv[3], "fpga") == 0) moduleMode = NISysCfgModuleProgramModeLabVIEWFpga;
    else if (strcmp(argv[3], "daq") == 0) moduleMode = NISysCfgModuleProgramModeRealtimeCpu;
    else return 1;

    nirtconfig_setAllModuleModes(session, moduleMode);

    NISysCfgCloseHandle(session);

    return status;
}

void nirtconfig_setAllModuleModes(NISysCfgSessionHandle session, NISysCfgModuleProgramMode moduleMode)
{
    NISysCfgEnumResourceHandle resourceHandle = NULL;
    NISysCfgFilterHandle filter = NULL;
    NISysCfgResourceHandle resource = NULL;
    char alias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    int status = 0;
 
    //Set up filter
    NISysCfgCreateFilter(session, &filter);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyConnectsToBusType, NISysCfgBusTypeCompactRio);

    //Find resource 
    NISysCfgFindHardware(session, NISysCfgFilterModeMatchValuesAll, filter, NULL, &resourceHandle);
    
    while ( (status = NISysCfgNextResource(session, resourceHandle, &resource)) == NISysCfg_OK) //Iterate through all modules
    {
        NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertUserAlias, 0, alias);
        NISysCfgSetResourceProperty(resource, NISysCfgResourcePropertyModuleProgramMode, moduleMode);
        printf("Setting Module Mode: %s\n", alias);
    }

    NISysCfgCloseHandle(resourceHandle);
    NISysCfgCloseHandle(resource);
}