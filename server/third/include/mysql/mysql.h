#pragma once

// Compatibility shim: some distro MySQL headers expose net_async_status only
// via plugin_auth_common.h but mysql.h uses it unconditionally.
#include <mysql/plugin_auth_common.h>
#include_next <mysql/mysql.h>
