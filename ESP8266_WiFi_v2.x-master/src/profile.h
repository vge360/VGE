#ifndef __PROFILE_H
#define __PROFILE_H

#if defined(ENABLE_PROFILE) && defined(ENABLE_DEBUG) //dòng này có nghĩa là nếu ENABLE_PROFILE và ENABLE_DEBUG đều được định nghĩa thì mới thực hiện các dòng lệnh bên dưới

#define Profile_Start(x) \ // dòng này có nghĩa là bắt đầu đo thời gian thực hiện của hàm x
  unsigned long profile ## x = millis() // dòng này có nghĩa là khai báo biến profile ## x có giá trị bằng thời gian hiện tại

#define Profile_End(x, max) \ // dòng này có nghĩa là kết thúc đo thời gian thực hiện của hàm x và in ra màn hình nếu thời gian thực hiện lớn hơn max
  unsigned long profile ## x ## Diff = millis() - profile ## x; \ // dòng này có nghĩa là khai báo biến profile ## x ## Diff có giá trị bằng thời gian thực hiện của hàm x
  if(profile ## x ## Diff > max) { \ // dòng này có nghĩa là nếu thời gian thực hiện của hàm x lớn hơn max thì in ra màn hình
    DBUGF(">> Slow " #x " %lums", profile ## x ## Diff);\  // dòng này có nghĩa là in ra màn hình thời gian thực hiện của hàm x
  }

#else // ENABLE_PROFILE

#define Profile_Start(x) 
#define Profile_End(x, min) //
#endif // ENABLE_PROFILE

#endif
