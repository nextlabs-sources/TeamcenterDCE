#ifndef NXL_HOOK_RMC_FIXES_H_INCLUDED
#define NXL_HOOK_RMC_FIXES_H_INCLUDED

BOOL fix_stat64_install();
void fix_stat64_uninstall();

//BOOL fix_QueryDirectoryFile_install();
//void fix_QueryDirectoryFile_uninstall();

BOOL fix_rpm_createfile_install();
void fix_rpm_createfile_uninstall();

BOOL fix_rpm_deletefile_install();
void fix_rpm_deletefile_uninstall();

BOOL fix_rpm_movefile_install();
void fix_rpm_movefile_uninstall();

#endif