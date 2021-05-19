#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <nisyscfg/nisyscfg.h>

//Function Declarations
int nirtconfig_find(int argc, char** argv);
int nirtconfig_getimage(int argc, char** argv);
void nirtconfig_printSystemInfo(NISysCfgSessionHandle session);
int status;

//Main
int main(int argc, char** argv)
{
    
    if (argc > 1) //Command passed as argument
    {
        std::string command = argv[1];

        if (command == "find")
        {
            status = nirtconfig_find(argc, argv);
        }

        else if (command == "getimage")
        {
            status = nirtconfig_getimage(argc, argv);
        }

        else
        {
            printf("No function: %s\n", argv[1]);
            status = 1;
        }
    }
    else
    {
        printf("No Function Passed\n");
        status = 1;
    }

    if (status != 0) printf("Error: %d\n", status);
    return status;
}

int nirtconfig_find(int argc, char** argv)
{

    NISysCfgEnumSystemHandle enumSystemHandle = NULL;
    NISysCfgSessionHandle session = NULL;
    char systemName[NISYSCFG_SIMPLE_STRING_LENGTH] = "";
    int status = 0;

    if (argc > 2) //IP address passed as argument, find specific target
    {
        char targetName[] = "";
        strcpy(targetName, argv[2]);

        status = NISysCfgInitializeSession(targetName, NULL, NULL, NISysCfgLocaleDefault, 
                                            NISysCfgBoolFalse, 10000, NULL, &session);

        if (status != 0)
        {
            printf("Target Not Found\n");
            return status;
        }
        
        printf("%-25s%-20s%-15s%s\n", "HOSTNAME", "IP ADDR", "MODEL", "SERIAL NUMBER");
        nirtconfig_printSystemInfo(session);

        status = NISysCfgCloseHandle(session);
        return status;
    }

    else //no arguments passed, find all available targets
    {
        status = NISysCfgFindSystems(NULL, NULL, NISysCfgBoolTrue, 
                            NISysCfgIncludeCachedResultsNone, NISysCfgSystemNameFormatHostname,
                            10000,NISysCfgBoolTrue,&enumSystemHandle);

        printf("%-25s%-20s%-15s%s\n", "HOSTNAME", "IP ADDR", "MODEL", "SERIAL NUMBER");

        while(status = NISysCfgNextSystemInfo(enumSystemHandle, systemName) == 0)
        {
            NISysCfgInitializeSession(systemName, NULL, NULL, NISysCfgLocaleDefault, 
                                            NISysCfgBoolFalse, 10000, NULL, &session);

            nirtconfig_printSystemInfo(session);
            status = NISysCfgCloseHandle(session);
        }
    }    

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

int nirtconfig_getimage(int argc, char** argv)
{
    int status = 0;

    if (argc > 2) //argument passed
    {
        NISysCfgSessionHandle session = NULL;
        char targetName[] = "";
        strcpy(targetName, argv[2]);

        status = NISysCfgInitializeSession(targetName, NULL, NULL, NISysCfgLocaleDefault, 
                                            NISysCfgBoolFalse, 10000, NULL, &session);

        if (status != 0)
        {
            printf("Unable to Connect to Target\n");
            return status;
        }

        printf("Getting Image: %s\n", targetName);

        char destination[256];
        getcwd(destination, sizeof(destination) );
        printf("Saving To: \"%s\"\n", destination);

        // status = NISysCfgGetSystemImageAsFolder2(session, NISysCfgBoolTrue, destination,
        //                                  NULL, 0, NULL, NISysCfgBoolTrue, NISysCfgBoolFalse);
    }

    else //no arguments passed
    {
        printf("No Target Selected\n");
        status = 1;
    }

    return status;
}