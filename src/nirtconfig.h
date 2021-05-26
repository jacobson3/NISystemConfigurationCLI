#include <nisyscfg/nisyscfg.h>

//Callable Functions
//Must be in the following form:
//int FUNCTION_NAME(int argc, char** argv);
int nirtconfig_find(int argc, char** argv);
int nirtconfig_getImage(int argc, char** argv);
int nirtconfig_setImage(int argc, char** argv);

//Subroutines
int nirtconfig_findSingleTarget(char *targetName);
int nirtconfig_findAllTargets();
void nirtconfig_printSystemInfo(NISysCfgSessionHandle session);
void nirtconfig_buildOutputDir(NISysCfgSessionHandle session, char* pathBuffer);