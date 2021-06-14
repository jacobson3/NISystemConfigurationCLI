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
} nirtFunctions[] = {
    { "find", nirtconfig_find },
    { "setimage", nirtconfig_setImage },
    { "getimage", nirtconfig_getImage },
    { "selftest", nirtconfig_selfTest },
    { "sethostname", nirtconfig_setHostname },
    { "setip", nirtconfig_setIpAddress },
    { "restart", nirtconfig_restartTarget },
    { "updatefirmware", nirtconfig_updateFirmware },
    { "findsn", nirtconfig_ipFromSerialNumber },
    { "setmode", nirtconfig_setModuleMode },
    { "listhw", nirtconfig_listHardware },
    { "format", nirtconfig_format },
    { "setalias", nirtconfig_setAlias },
    { NULL, NULL }
};

int main(int argc, char** argv)
{
    int status = 0;

    if (argc > 1) //Command passed as argument
    {
        int i = 0;

        while (nirtFunctions[i].name != NULL) //Iterate through function lookup table
        {
            if (strcmp(nirtFunctions[i].name, argv[1]) == 0) //Check if command matches function in table
            {
                status = (*nirtFunctions[i].fpointer)(argc, argv);
                break;
            }
            i++;
        }

        if (nirtFunctions[i].name == NULL)
            printf("Invalid Command: %s\n", argv[1]);
    }

    else //No arguments passed
    {
        printf("No Function Passed\n");
        status = 1;
    }

    if (status != 0)
        nirtconfig_printStatusInfo(status);

    return status;
}

void nirtconfig_printStatusInfo(int status)
{
    char* detailedResults = NULL;

    NISysCfgGetStatusDescription(NULL, (NISysCfgStatus)status, &detailedResults);

    printf("Error: %d\n", status);
    if (strcmp(detailedResults, "") != 0)
        printf("%s\n", detailedResults);

    NISysCfgFreeDetailedString(detailedResults);
}

int nirtconfig_find(int argc, char** argv)
{
    int status = 0;

    if (argc > 2) //IP address passed as argument, find specific target
    {
        status = nirtconfig_findSingleTarget(argv[2]);
    }

    else //no arguments passed, find all available targets
    {
        status = nirtconfig_findAllTargets();
    }

    return status;
}

int nirtconfig_findSingleTarget(char* targetName)
{
    int status = 0;
    NISysCfgSessionHandle session = NULL;

    status = NISysCfgInitializeSession(targetName, NULL, NULL, NISysCfgLocaleDefault,
                                       NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0)
        return status; //Error initializeing session

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
                                 10000, NISysCfgBoolTrue, &enumSystemHandle);

    printf("%-35s%-20s%-15s%s\n", "HOSTNAME", "IP ADDR", "MODEL", "SERIAL NUMBER");

    while (NISysCfgNextSystemInfo(enumSystemHandle, systemName) == NISysCfg_OK) //Iterate through systems found
    {
        NISysCfgInitializeSession(systemName, NULL, NULL, NISysCfgLocaleDefault,
                                  NISysCfgBoolFalse, 10000, NULL, &session);

        nirtconfig_printSystemInfo(session);
        NISysCfgCloseHandle(session);
    }

    NISysCfgCloseHandle(enumSystemHandle);

    return status;
}

void nirtconfig_printSystemInfo(NISysCfgSessionHandle session)
{
    char hostname[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char ipaddr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char model[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char serialNumber[NISYSCFG_SIMPLE_STRING_LENGTH] = "";

    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertyHostname, hostname);
    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertyIpAddress, ipaddr);
    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertyProductName, model);
    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertySerialNumber, serialNumber);

    printf("%-35s%-20s%-15s%s\n", hostname, ipaddr, model, serialNumber);
}

int nirtconfig_getImage(int argc, char** argv)
{
    if (argc != 3) //Check for correct number of arguments
    {
        printf("Error Expecting Arguments: getimage <TARGETNAME>\n");
        return 0;
    }

    int status = 0;
    NISysCfgSessionHandle session = NULL;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault,
                                       NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0)
        return status; //Error initializeing session

    char destination[256] = "";
    nirtconfig_buildOutputDir(session, destination);

    printf("Getting Image: %s\nSaving To: \"%s\"\n", argv[2], destination);

    status = NISysCfgGetSystemImageAsFolder2(session, NISysCfgBoolTrue, destination,
                                             NULL, 0, NULL, NISysCfgBoolTrue, NISysCfgBoolFalse);

    return status;
}

void nirtconfig_buildOutputDir(NISysCfgSessionHandle session, char* pathBuffer)
{
    char hostname[NISYSCFG_SIMPLE_STRING_LENGTH] = "";

    getcwd(pathBuffer, 256);
    strcat(pathBuffer, "/");

    NISysCfgGetSystemProperty(session, NISysCfgSystemPropertyHostname, hostname);
    strcat(pathBuffer, hostname);
}

int nirtconfig_setImage(int argc, char** argv)
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

    if (status != 0)
        return status; //Error initializeing session

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

    if (status != 0)
        return status; //Error initializeing session

    NISysCfgEnumResourceHandle resourceHandle = NULL;
    NISysCfgResourceHandle resource = NULL;
    NISysCfgFilterHandle filter = NULL;

    NISysCfgCreateFilter(session, &filter);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertySlotNumber, 0);

    status = NISysCfgFindHardware(session, NISysCfgFilterModeAllPropertiesExist, filter, NULL, &resourceHandle);

    printf("Running Self Tests...\n");
    printf("%-40s%-20s%-15s%s\n", "RESOURCE NAME", "PRODUCT NAME", "PASS/FAIL", "DETAILED RESULTS");
    while (NISysCfgNextResource(session, resourceHandle, &resource) == NISysCfg_OK) //Iterate through all hardware resources
    {
        nirtconfig_printSelfTestResults(resource);
        NISysCfgCloseHandle(resource);
    }

    NISysCfgCloseHandle(resourceHandle);
    NISysCfgCloseHandle(session);

    return status;
}

void nirtconfig_printSelfTestResults(NISysCfgResourceHandle resource)
{
    char productName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char resourceName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char alias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char passFail[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
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

    //print results using resource's alias if available, resource name if not
    if (strlen(alias))
        printf("%-40s%-20s%-15s%s\n", alias, productName, passFail, detailedResults);
    else
        printf("%-40s%-20s%-15s%s\n", resourceName, productName, passFail, detailedResults);

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

    if (status != 0)
        return status; //Error initializeing session

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

    if (status != 0)
        return status; //Error initializeing session

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

    if (status != 0)
        return status; //Error initializeing session

    char ipAddr[NISYSCFG_SIMPLE_STRING_LENGTH] = "";

    printf("Restarting...\n");

    status = NISysCfgRestart(session, NISysCfgBoolTrue, NISysCfgBoolFalse, NISysCfgBoolFalse, 120000, ipAddr);
    if (status == 0)
        printf("Restarted With IP Address: %s\n", ipAddr);

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

    status = NISysCfgInitializeSession(argv[argc - 2], username, password, NISysCfgLocaleDefault,
                                       NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0)
        return status; //Error initializeing session

    NISysCfgResourceHandle resource = NULL;
    NISysCfgFirmwareStatus firmwareStatus;
    char* detailedResults = NULL;

    if (nirtconfig_findFirmwareResource(session, &resource) == NISysCfg_OK) //Get hardware resource and set firmware
    {
        printf("Updating Firmware...\nTarget: %s\nFirmware: %s\n", argv[argc - 2], argv[argc - 1]);
        status = NISysCfgUpgradeFirmwareFromFile(resource, argv[argc - 1], NISysCfgBoolTrue, NISysCfgBoolTrue,
                                                 NISysCfgBoolTrue, &firmwareStatus, &detailedResults);

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

    while ((flag = getopt(argc, argv, "u:p:")) != -1) //search through incoming arguments
    {
        switch (flag)
        {
            case 'u':
                strcpy(username, optarg);
                break;

            case 'p':
                strcpy(password, optarg);
                break;

            default:
                break;
        }
    }
}

int nirtconfig_findFirmwareResource(NISysCfgSessionHandle session, NISysCfgResourceHandle* resource)
{
    int status = 0;
    NISysCfgEnumResourceHandle resourceHandle = NULL;
    NISysCfgFilterHandle filter = NULL;

    NISysCfgCreateFilter(session, &filter);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertySupportsFirmwareUpdate, NISysCfgBoolTrue);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyResourceName, "system");

    NISysCfgFindHardware(session, NISysCfgFilterModeMatchValuesAll, filter, NULL, &resourceHandle);
    status = NISysCfgNextResource(session, resourceHandle, resource);

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
                                 10000, NISysCfgBoolTrue, &enumSystemHandle);

    while (NISysCfgNextSystemInfo(enumSystemHandle, systemIP) == NISysCfg_OK) //Iterate through systems found
    {
        NISysCfgInitializeSession(systemIP, NULL, NULL, NISysCfgLocaleDefault,
                                  NISysCfgBoolFalse, 10000, NULL, &session);

        NISysCfgGetSystemProperty(session, NISysCfgSystemPropertySerialNumber, serialNumber);
        NISysCfgCloseHandle(session);

        if ((strcmp(argv[2], serialNumber)) == 0) //Found correct target
        {
            NISysCfgCloseHandle(enumSystemHandle);
            printf("%s\n", systemIP);
            return 0;
        }
    }

    printf("Target With SN %s Not Found\n", argv[2]);
    NISysCfgCloseHandle(enumSystemHandle);

    return status;
}

int nirtconfig_setModuleMode(int argc, char** argv)
{
    if (argc < 4) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: setmode <TARGETNAME> <scan|fpga|daq>\n");
        return 0;
    }

    //Get module programming mode from input
    NISysCfgModuleProgramMode moduleMode = NISysCfgModuleProgramModeNone;
    if (strcmp(argv[3], "scan") == 0)
        moduleMode = NISysCfgModuleProgramModeRealtimeScan;
    else if (strcmp(argv[3], "fpga") == 0)
        moduleMode = NISysCfgModuleProgramModeLabVIEWFpga;
    else if (strcmp(argv[3], "daq") == 0)
        moduleMode = NISysCfgModuleProgramModeRealtimeCpu;
    else
    {
        printf("Programming mode \"%s\" invalid. Choose from programming modes scan, fpga, or daq.\n", argv[3]);
        return 0;
    }

    NISysCfgSessionHandle session = NULL;
    int status = 0;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault,
                                       NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0)
        return status; //Error initializing session

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
    char productName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    NISysCfgBool changesRequired;
    char* detailedResults;
    int status = 0;

    NISysCfgCreateFilter(session, &filter);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertyConnectsToBusType, NISysCfgBusTypeCompactRio);

    NISysCfgFindHardware(session, NISysCfgFilterModeMatchValuesAll, filter, NULL, &resourceHandle);

    while ((NISysCfgNextResource(session, resourceHandle, &resource)) == NISysCfg_OK) //Iterate through all modules
    {
        NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertUserAlias, 0, alias);
        NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyProductName, productName);
        printf("Setting Module Mode: %s (%s)\n", alias, productName);

        NISysCfgSetResourceProperty(resource, NISysCfgResourcePropertyModuleProgramMode, moduleMode);

        NISysCfgSaveResourceChanges(resource, &changesRequired, &detailedResults);
        NISysCfgCloseHandle(resource);
    }

    NISysCfgFreeDetailedString(detailedResults);
    NISysCfgCloseHandle(resourceHandle);
}

int nirtconfig_listHardware(int argc, char** argv)
{
    if (argc != 3) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: listhw <TARGETNAME>\n");
        return 0;
    }

    NISysCfgSessionHandle session = NULL;
    int status = 0;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault,
                                       NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0)
        return status; //Error initializing session

    NISysCfgEnumResourceHandle resourceHandle = NULL;
    NISysCfgResourceHandle resource = NULL;
    NISysCfgFilterHandle filter = NULL;

    NISysCfgCreateFilter(session, &filter);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertySlotNumber, 0);

    status = NISysCfgFindHardware(session, NISysCfgFilterModeAllPropertiesExist, filter, NULL, &resourceHandle);

    printf("%-10s%-15s%s\n", "SLOT", "MODULE", "ALIAS");
    while (NISysCfgNextResource(session, resourceHandle, &resource) == NISysCfg_OK) //Iterate through all hardware resources
    {
        nirtconfig_printHardwareList(resource);
        NISysCfgCloseHandle(resource);
    }

    NISysCfgCloseHandle(resourceHandle);
    NISysCfgCloseHandle(session);

    return status;
}

void nirtconfig_printHardwareList(NISysCfgResourceHandle resource)
{
    char productName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char alias[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char serialNumber[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    int slotNumber = 0;
    int status = 0;

    NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertyProductName, productName);
    NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertySlotNumber, &slotNumber);
    NISysCfgGetResourceIndexedProperty(resource, NISysCfgIndexedPropertyExpertUserAlias, 0, alias);

    NISysCfgGetResourceProperty(resource, NISysCfgResourcePropertySerialNumber, serialNumber);

    // Filter out resources without a SN
    // PXIe controllers will list GPIB ports as a unique resource in slot 1
    if (strcmp(serialNumber, "") != 0)
        printf("%-10d%-15s%s\n", slotNumber, productName, alias);
}

int nirtconfig_format(int argc, char** argv)
{
    if (argc < 3) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: format <TARGETNAME>\n");
        return 0;
    }

    NISysCfgSessionHandle session = NULL;
    char username[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    char password[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    int status = 0;

    nirtconfig_getCredentials(argc, argv, username, password);

    status = NISysCfgInitializeSession(argv[argc - 1], username, password, NISysCfgLocaleDefault,
                                       NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0)
        return status; //Error initializeing session

    printf("Formatting...\n");

    status = NISysCfgFormat(session, NISysCfgBoolTrue, NISysCfgBoolTrue,
                            NISysCfgFileSystemDefault, NISysCfgPreservePrimaryResetOthers, 120000);

    return status;
}

int nirtconfig_setAlias(int argc, char** argv)
{
    if (argc != 5) //Check for correct number of incoming arguments
    {
        printf("Error Expecting Arguments: setalias <TARGETNAME> <SLOT> <NEW_ALIAS>\n");
        return 0;
    }

    NISysCfgSessionHandle session = NULL;
    int status = 0;

    status = NISysCfgInitializeSession(argv[2], NULL, NULL, NISysCfgLocaleDefault,
                                       NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0)
        return status; //Error initializeing session

    NISysCfgEnumResourceHandle resourceHandle = NULL;
    NISysCfgResourceHandle resource = NULL;
    NISysCfgFilterHandle filter = NULL;
    NISysCfgBool nameExisted;
    NISysCfgResourceHandle overwrittenResource = NULL;
    int slotNumber = 0;
    sscanf(argv[3], "%d", &slotNumber); //convert char argument to int

    NISysCfgCreateFilter(session, &filter);
    NISysCfgSetFilterProperty(filter, NISysCfgFilterPropertySlotNumber, slotNumber);

    NISysCfgFindHardware(session, NISysCfgFilterModeMatchValuesAll, filter, NULL, &resourceHandle);
    NISysCfgNextResource(session, resourceHandle, &resource);

    status = NISysCfgRenameResource(resource, argv[4], NISysCfgBoolFalse, NISysCfgBoolTrue, &nameExisted, &overwrittenResource);

    NISysCfgCloseHandle(resourceHandle);
    NISysCfgCloseHandle(session);

    return status;
}