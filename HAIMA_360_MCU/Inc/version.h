
#ifndef __VERSION_H__
#define __VERSION_H__

#include <version.h>

// software version base define
#define __VERSION(ma, md, mi)	(u32)(((mi) << 16) | ((md) << 8) | (ma))


// software version
#define APP_VER_BIN				__VERSION(5, 0, 1)
#define BOOT_VER_BIN			__VERSION(1, 0, 1)
#define MCU_BOOT_VER        "AVM-MCU-VER-0.1.0.1"     //mcu  boot 版本 与 BOOT_VER_BIN 同步 用于boot升级


#endif /* end of __VERSION_H__ */

