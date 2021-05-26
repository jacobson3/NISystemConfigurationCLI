#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <nisyscfg/nisyscfg.h>

//Function Declarations
int nirtconfig_find(int argc, char** argv);
int nirtconfig_getImage(int argc, char** argv);
int nirtconfig_setImage(int argc, char** argv);
int nirtconfig_findSingleTarget(char *targetName);
int nirtconfig_findAllTargets();
void nirtconfig_printSystemInfo(NISysCfgSessionHandle session);
void nirtconfig_buildOutputDir(NISysCfgSessionHandle session, char* pathBuffer);

static const struct
{
    const char* const name;
    int (*fpointer)(int argc, char** argv);
} nirtFunctions[] =
{
    {"find", nirtconfig_find},
    {"setimage", nirtconfig_setImage},
    {"getimage", nirtconfig_getImage},
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
        char targetName[] = "";
        strcpy(targetName, argv[2]);

        status = nirtconfig_findSingleTarget(targetName);

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
    
    printf("%-25s%-20s%-15s%s\n", "HOSTNAME", "IP ADDR", "MODEL", "SERIAL NUMBER");
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

    status = NISysCfgFindSystems(NULL, NULL, NISysCfgBoolTrue, 
                            NISysCfgIncludeCachedResultsNone, NISysCfgSystemNameFormatHostname,
                            10000,NISysCfgBoolTrue,&enumSystemHandle);

    printf("%-25s%-20s%-15s%s\n", "HOSTNAME", "IP ADDR", "MODEL", "SERIAL NUMBER");

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

    printf("%-25s%-20s%-15s%s\n", hostname, ipaddr, model, serialNumber);
}

int nirtconfig_getImage(int argc, char** argv)
{
    int status = 0;

    if (argc > 2) //argument passed
    {
        NISysCfgSessionHandle session = NULL;
        char targetName[256] = "";
        strcpy(targetName, argv[2]);

        status = NISysCfgInitializeSession(targetName, NULL, NULL, NISysCfgLocaleDefault, 
                                            NISysCfgBoolFalse, 10000, NULL, &session);

        if (status != 0) //error opening session
        {
            printf("Unable to Connect to Target\n");
            return status;
        }

        printf("Getting Image: %s\n", targetName);

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
    char targetName[256] = "";
    strcpy(targetName, argv[2]);
    char imagePath[256] = "";
    strcpy(imagePath, argv[3]);

    status = NISysCfgInitializeSession(targetName, NULL, NULL, NISysCfgLocaleDefault, 
                                        NISysCfgBoolFalse, 10000, NULL, &session);

    if (status != 0) //error opening session
    {
        printf("Unable to Connect to Target: %s\n", targetName);
        return status;
    }

    printf("Imaging Target: %s\nImage Used: %s\n", targetName, imagePath);

    status = NISysCfgSetSystemImageFromFolder2(session, NISysCfgBoolTrue, imagePath, "", 0, NULL, 
                                            NISysCfgBoolFalse, NISysCfgPreservePrimaryResetOthers);
    
    status = NISysCfgCloseHandle(session);
    return status;
}