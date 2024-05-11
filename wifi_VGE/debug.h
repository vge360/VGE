#ifndef __DEBUG_H
#define __DEBUG_H

#include <core_version.h>


#define TEXTIFY(A) #A
#define ESCAPEQUOTE(A) TEXTIFY(A)

#ifdef ENABLE_DEBUG


#define DEBUG_BEGIN(speed)  DEBUG_PORT.begin(speed)

#define DBUGF(format, ...)  DEBUG_PORT.printf(format "\n", ##__VA_ARGS__)
//#endif

#define DBUG(...)           DEBUG_PORT.print(__VA_ARGS__)
#define DBUGLN(...)         DEBUG_PORT.println(__VA_ARGS__)
#define DBUGVAR(x, ...)     do { DEBUG_PORT.print(ESCAPEQUOTE(x) " = "); DEBUG_PORT.println(x, ##__VA_ARGS__); } while (false)
//DBUGVAR in giá trị của biến cùng tên biến. Ví dụ, DBUGVAR(temp) in ra "temp = [giá trị của temp]".
#else
//Nếu ENABLE_DEBUG không được định nghĩa, các hàm debug sẽ không làm gì cả ngoài việc khởi tạo cổng Serial.
#define DEBUG_BEGIN(speed)  DEBUG_PORT.begin(speed)
#define DBUGF(...)
#define DBUG(...)
#define DBUGLN(...)
#define DBUGVAR(...)

#endif // DEBUG
//Kiểm tra nếu DEBUG_SERIAL1 được định nghĩa và thông báo lỗi, yêu cầu sử dụng DEBUG_PORT để đặt cổng Serial.
#ifdef DEBUG_SERIAL1
#error DEBUG_SERIAL1 defiend, please use -DDEBUG_PORT=Serial1 instead
#endif

#ifndef DEBUG_PORT
#define DEBUG_PORT Serial1
#endif
#define DEBUG DEBUG_PORT

#endif // __DEBUG_H
