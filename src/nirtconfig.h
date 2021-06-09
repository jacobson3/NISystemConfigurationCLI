#include <nisyscfg/nisyscfg.h>

struct hwNode //Node for creating linked list of hardware modules
{
    int slot;
    char *alias;
    char *productName;
    struct hwNode *next;
};


//Callable Functions
//Must be in the following form:
//int FUNCTION_NAME(int argc, char** argv);
int nirtconfig_find(int argc, char** argv);
int nirtconfig_getImage(int argc, char** argv);
int nirtconfig_setImage(int argc, char** argv);
int nirtconfig_selfTest(int argc, char** argv);
int nirtconfig_setHostname(int argc, char** argv);
int nirtconfig_setIpAddress(int argc, char** argv);
int nirtconfig_restartTarget(int argc, char** argv);
int nirtconfig_updateFirmware(int argc, char** argv);
int nirtconfig_ipFromSerialNumber(int argc, char** argv);
int nirtconfig_setModuleMode(int argc, char** argv);
int nirtconfig_listHardware(int argc, char** argv);

//Subroutines
void nirtconfig_printStatusInfo(int status);
int nirtconfig_findSingleTarget(char *targetName);
int nirtconfig_findAllTargets();
void nirtconfig_printSystemInfo(NISysCfgSessionHandle session);
void nirtconfig_buildOutputDir(NISysCfgSessionHandle session, char* pathBuffer);
void nirtconfig_printSelfTestResults(NISysCfgResourceHandle resource);
void nirtconfig_getCredentials(int argc, char** argv, char* username, char* password);
int nirtconfig_findFirmwareResource(NISysCfgSessionHandle session, NISysCfgResourceHandle *resource);
void nirtconfig_setAllModuleModes(NISysCfgSessionHandle session, NISysCfgModuleProgramMode moduleMode);
void nirtconfig_printHardwareList(NISysCfgResourceHandle resource);