#ifndef _EMONESP_H
#define _EMONESP_H

// -------------------------------------------------------------------
// General support code used by all modules
// -------------------------------------------------------------------

#include "debug.h"
#include "profile.h"

#ifndef RAPI_PORT
#ifdef ESP32
#define RAPI_PORT Serial1
#elif defined(ESP8266)
#define RAPI_PORT Serial
#else
#error Platform not supported
#endif
#endif


// Đặt giá trị mặc định cho điện áp nếu không được xác định trước
#ifndef DEFAULT_VOLTAGE
#define DEFAULT_VOLTAGE  240
#endif
// Kiểm tra xem có sử dụng tỷ lệ cảm biến không
#ifdef NO_SENSOR_SCALING
// Nếu không sử dụng tỷ lệ cảm biến, tất cả các hệ số sẽ là 1.0
#ifndef VOLTS_SCALE_FACTOR
#define VOLTS_SCALE_FACTOR  1.0
#endif

#ifndef AMPS_SCALE_FACTOR
#define AMPS_SCALE_FACTOR   1.0
#endif

#ifndef TEMP_SCALE_FACTOR
#define TEMP_SCALE_FACTOR   1.0
#endif

#else
// Nếu có sử dụng tỷ lệ cảm biến
#ifndef VOLTS_SCALE_FACTOR
#define VOLTS_SCALE_FACTOR  1.0
#endif
// Điều chỉnh hệ số tỷ lệ cho dòng điện và nhiệt độ
#ifndef AMPS_SCALE_FACTOR
#define AMPS_SCALE_FACTOR   1000.0
#endif

#ifndef TEMP_SCALE_FACTOR
#define TEMP_SCALE_FACTOR   10.0
#endif

#endif

#endif // _EMONESP_H
