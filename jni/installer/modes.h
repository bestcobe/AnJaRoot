/*
 * Copyright 2013 Simon Brakhane
 *
 * This file is part of AnJaRoot.
 *
 * AnJaRoot is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * AnJaRoot is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * AnJaRoot. If not, see http://www.gnu.org/licenses/.
 */

#ifndef _ANJAROOT_INSTALLER_MODES_H_
#define _ANJAROOT_INSTALLER_MODES_H_

#include <string>

namespace modes {
    enum OperationMode {
        InvalidMode,
        InstallMode,
        UninstallMode,
        CheckMode,
        RecoveryInstallMode,
        RebootRecoveryMode,
        RebootSystemMode,
        HelpMode,    // doesn't belong here, but I don't want to special
        VersionMode  // special case them even more than I did already...
    };

    enum ReturnCode {
        OK,
        FAIL
    };

    ReturnCode install(const std::string& libpath,
            const std::string& daemonpath, const std::string& apkpath);
    ReturnCode uninstall();
    ReturnCode check();
    ReturnCode recoveryInstall(const std::string& apkpath);
    ReturnCode rebootIntoRecovery();
    ReturnCode rebootSystem();
}

#endif
