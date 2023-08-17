/**
 * 北斗三号短报文终端普通短报文发送接口
 * bdrd_esp32.h
 * 支持ESP32平台
 * Serial2接口为短报文终端数据传输串口 16-RX2，17-TX2
 * Serial接口为调试信息输出接口 1-TX0，3-RX0
*/

// 区域短报文基本流程：
//
//1. 开机上电启动、1S后进行操作
//2. 发送ICR，查询本机信息
//3. 监听ICP，等待查询结果，未插卡或者其他异常进行提示，状态正常允许进行后面操作
//4. 监听PWI语句，等待正确获得时间信息，允许进行后续操作
//5. 发送TCQ消息，看看能否进展到这步
//6. 监听FKI，判断发送有没有其他异常信息，进行提示

#ifndef _BDRD_ESP32_H
#define _BDRD_ESP32_H

// 接口声明
/**
  @brief   短报文发送函数。发送形式可以是“汉字”形式或者“代码”形式
           当“汉字”形式时，类型参数type赋值'1'，内容参数content为汉字的GB2312机器码
           当“代码”形式时，类型参数type赋值'2'，内容参数content为数字字符串
  @param   id       发送目标ID号码（终端号、集团号）
  @param   type     短报文发送类型（'1'代表汉字类型，'2'代表代码类型）
  @param   content  发送内容参数（字节数组形式）
                    当为“汉字”类型时，发送数据必须为GB2312机器码编码（无论汉字英文还是数字），不能传输ASCII编码
                    每个字符由2字节（16bit）组成，高位在前，开发时如用16bit结构，需要注意字节存储大小端的问题
                    ESP32为小端处理器，低字节在低位地址，整理成字节数组时，需要通过BSWAP_16工具进行编码转换。
                    当为“代码”类型时，接口填写十进制数字字符串，如"1234567890"
                    系统会自动编码成8421 BCD编码进行发送，4bit表示一个十进制数字
                    注意字符串只能填写0-9的十进制阿拉伯数值，不能有其他任何字符
                    如果需要传输其他数据，需要利用数字进行二次编码设计。
  @return  发送结果。0表示发送成功，1-12表示发送失败。
           每个数字代表一种结果，具体含义如下：
             0 - 正常
             1 - 无卡
             2 - ICP超时
             3 - 无时间
             4 - 无信号
             5 - PWI超时
             6 - 频率未到
             7 - 发射抑制
             8 - 发射静默
             9 - 功率未锁定
             10 - 未检测到IC模块信息
             11 - 权限不足
             12 - FKI超时
           可以通过调用showError函数，将错误原因通过调试串口Serial输出。
*/
int sendSMS(char* id, char type, char* content);

/**
  @brief   本机及IC模块信息查询。发送ICR信息,查询设备和北斗卡的基础信息："$CCICR,0,00*"
           发送后，通过监听ICP消息获取相应信息
  @return  无
*/
void sendICR();

/**
  @brief   IC模块信息查询输出。在ICR信息发送后，等待设备返回的信息
  @return  查询结果：
              0 - 正常
              1 - 无卡
              2 - ICP超时
*/
int waitForICP();

/**
  @brief   设备每秒串口发送PWI消息，波束跟踪信息。通过解析该消息，判断波束状态是否具备发送条件
  @return  监听结果：
              0 - 正常
              3 - 无时间
              4 - 无信号
              5 - PWI超时
*/
int waitForPWI();

/**
  @brief   报文通信申请，输入语句，用于设置终端发送报文通信申请。
           当“汉字”形式时，类型参数type赋值'1'，内容参数content为汉字的GB2312机器码
           当“代码”形式时，类型参数type赋值'2'，内容参数content为数字字符串
           格式：$CCTCQ,x.x,x,x.x,x,x,x,c-c,x.x*hh <CR><LF>
           发送后，通过监听FKI消息得到反馈
  @param   targetId 发送目标ID号码（终端号、集团号）
  @param   type     短报文发送类型（'1'代表汉字类型，'2'代表代码类型）
  @param   content  发送内容参数（字节数组形式）
                    当为“汉字”类型时，发送数据必须为GB2312机器码编码（无论汉字英文还是数字），不能传输ASCII编码
                    每个字符由2字节（16bit）组成，高位在前，开发时如用16bit结构，需要注意字节存储大小端的问题
                    ESP32为小端处理器，低字节在低位地址，整理成字节数组时，需要通过BSWAP_16工具进行编码转换。
                    当为“代码”类型时，接口填写十进制数字字符串，如"1234567890"
                    系统会自动编码成8421 BCD编码进行发送，4bit表示一个十进制数字
                    注意字符串只能填写0-9的十进制阿拉伯数值，不能有其他任何字符
                    如果需要传输其他数据，需要利用数字进行二次编码设计。
  @return  无
*/
void sendTCQ(char* targetId, char type, char* content);

/**
  @brief   TCQ发射申请反馈状态，输出语句。设备向用户反馈入站发射类型，成功与否状态及原因等。
           格式：$BDFKI, hhmmss,aaa,c,x,xxxx *hh <CR><LF>
  @return  返回结果：
              0 - 正常
              6 - 频率未到
              7 - 发射抑制
              8 - 发射静默
              9 - 功率未锁定
              10 - 未检测到IC模块信息
              11 - 权限不足
              12 - FKI超时
*/
int waitForFKI();

/**
  @brief   校验码生成。根据内容字符串生成异或校验码字符串
  @param   str      校验的字符串
  @param   len      校验的字符串长度
  @return  校验后生成的2字节校验码字符串
*/
char* getCheck(const char *str, int len);

/**
  @brief   英文数字符号字符串ASCII转GB2312全角宽字符（不包含中文）
  @param   str      ASCII字符编码字符串
  @return  转换成GB2312全角字符的字符串
*/
char* getWCharString(char* str);

/**
  @brief   整型数字转GB2312全角宽字符
  @param   num      需要转换的整数
  @return  转换成GB2312全角字符的字符串
*/
char* getWCharString(int num);

/**
  @brief   浮点数字转GB2312全角宽字符
  @param   num      需要转换的浮点数字
  @param   declen   小数点后保留位数
  @return  转换成GB2312全角字符的字符串
*/
char* getWCharString(float num, int declen);

/**
  @brief   错误原因打印。将错误代码编号解析成原因文字，通过Seriial串口打印出来
  @param   result   错误代码
  @return  无
*/
void showError(int result);

/**
  @brief   16bit大端小端字节顺序处理工具
  @param   x     需要大小端转换的16bit数字
  @return  转换后的16bit数字
*/
#define BSWAP_16(x) \
  (uint16_t)((((uint16_t)(x) & 0x00FF) << 8) | (((uint16_t)(x) & 0xFF00) >> 8))

// 全局变量初始化
char msg[200] = {0};

// 短报文发送函数
int sendSMS(char* id, char type, char* content) {
  Serial2.end();
  Serial2.begin(115200);
  
  sendICR();
  int result = waitForICP();
  if (result == 0) {
    result = waitForPWI();
    if (result == 0) {
      sendTCQ(id, type, content);
      result = waitForFKI();
    } 
  } 
  return result;
}

// 本机及IC模块信息查询
void sendICR() {
  // 发送ICR获取设备信息
  char cmdstr[] = "CCICR,0,00";
  char* checkCodeStr = getCheck(cmdstr, strlen(cmdstr));
  sprintf(msg, "$%s*%s\r\n", cmdstr, checkCodeStr);
  Serial2.write(msg);
}

// IC模块信息查询输出
int waitForICP() {
  int result = 0; // 0-正常； 1-无卡； 2-ICP超时
  uint32_t overTime = millis();
  
  while (1){
    if(Serial2.available()){
      char recvMsg[200] = {0};
      size_t len = Serial2.readBytesUntil('\n', recvMsg, 200);

      if (len >= 6) {
        memset(msg, 0, 200);
        strncpy(msg, recvMsg+1, len-5); // 截取$和*之间字符串
      } else {
        // 收到的错误信息
        continue;
      }
      
      unsigned int i = 0;
      char *q = msg;
      char *p;

      p = strsep(&q, ",");
//      if (p && 0 == strcmp(p, "BDICP")) {
      if (p && NULL != strstr(p, "BDICP")) {
        while (i < 23){
          p = strsep(&q, ",");
          if (NULL == p) break;
          if (i == 0){  // 用户地址
//            Serial.println(String("id:") + p);
            if (p && 0 == strcmp(p, "0")) { // 无卡ID信息
              result = 1;
            } else {
              result = 0;
            }
          } else if (i == 3) {
//            Serial.println(String("bd") + p); // 北斗系统号
          } else if (i == 4) {
//            Serial.println(String("service start:") + p);
          } else if (i == 14) {
//            Serial.println(String("lengh level:") + p);
          }
          i++;
        }
//        Serial.println(recvMsg);
        break;
      }
    }
    
    if (millis() - overTime >= 2000) {
      result = 2;
      break;
    }
  }
  return result;
}

// 波束跟踪状态，输出语句
int waitForPWI() {
  // 示例：
  //$BDPWI,
  //023628.00,//时间
  //00,//北斗一代二代信号数目
  //04,//北斗三代信号数目
  //2,51,44,0, //信号通道2,三个信号质量，看第一个51就行了
  //4,50,43,0,
  //11,43,36,0,
  //16,51,44,0
  //*42
  int result = 0; // 0-正常； 3-无时间； 4-无信号； 5-PWI超时
  uint32_t overTime = millis();
  
  while (1){
    if(Serial2.available()){
      char recvMsg[200];
      size_t len = Serial2.readBytesUntil('\n', recvMsg, 200);
      if (len >= 6) {
        memset(msg, 0, 200);
        strncpy(msg, recvMsg+1, len-5); // 截取$和*之间字符串
      } else {
        // 收到的错误信息
        continue;
      }
      
      unsigned int i = 0;
      char *q = msg;
      char *p;

      p = strsep(&q, ",");
      if (p && 0 == strcmp(p, "BDPWI")) {
        // 返回波束跟踪状态
        while (i < 3){
          p = strsep(&q, ",");
          if (NULL == p) break;
          if (i == 0){  // 时间信息
            if (p && 0 == strcmp(p, "000000.00")) { // 没有获取到时间信息
              result = 3;
            } else {
              result = 0;
            }
          } else if (i == 2) {
            if (p && 0 == strcmp(p, "00")) {  // 没有波束信号
              result = 4;
            } else {
              result = 0;
            }
          }
          i++;
        }
//        Serial.println(recvMsg);
        break;
      }
    }
    
    if (millis() - overTime >= 2000) {
      result = 5;
      break;
    }
  }
  return result;
}

// 报文通信申请
void sendTCQ(char* targetId, char type, char* content) {
  // 星宇芯联消息格式：
  // ID - 短报文通信收信方ID
  // 频点（1〜5： Lf0-Lf4；）
  // 入站确认申请(1-不需确认；2-需确认)
  // 编码类别（1： 汉字； 2： 代码； 3： 混编； 4： 压缩汉字；5： 压缩代码。）
  // 通信数据(1.编码类别为“1” 时,传输内容为计算机内码，每个汉字 16bit,高位在前；2.编码类别为“2” 时，传输内容为 ASCII 码字符，如代码 8 以 ASCII 码字符'8' (HEX38)表示；当编码类别为“3” 时,传输内容为汉字代码混合，输出的BCD 码起始位“A4” ；)
  // 报文通信频度(0 为单次，关闭连续在 RMO 里关。)

  char cmdstr[200];
  sprintf(cmdstr, "CCTCQ,%s,2,2,%c,%s,0", targetId, type, content);
  char* checkCodeStr = getCheck(cmdstr, strlen(cmdstr));
  sprintf(msg, "$%s*%s\r\n", cmdstr, checkCodeStr);
  Serial.write(msg);
  Serial2.write(msg);
}

// 发射申请反馈状态
int waitForFKI() {
  // 等待回复信息，判断发送成功与否
  // 解析FKI语句，如：
  // $BDFKI,000000,TCQ,Y,5,0*74
  // 指令发射时间（时分秒）
  // 入站发射类型（TCQ-报文通信申请）
  // 发射情况（Y-发射成功；N-发射失败，发射情况未 Y 时，失败原因为 0 或置空，剩余时间为 0000 或置空）
  // 失败原因（1-频率未到；2-发射抑制；3-发射静默；4- 功率未锁定；5-未检测到IC模块信息；6-权限不足。）
  // 剩余时间

  int result = 0; // 0-正常；6-频率未到；7-发射抑制；8-发射静默；9-功率未锁定；10-未检测到IC模块信息；11-权限不足； 12-FKI超时
  uint32_t overTime = millis();
  
  while (1){
    if(Serial2.available()){
      char recvMsg[200];
      size_t len = Serial2.readBytesUntil('\n', recvMsg, 200);
      if (len >= 6) {
        memset(msg, 0, 200);
        strncpy(msg, recvMsg+1, len-5); // 截取$和*之间字符串
      } else {
        // 收到的错误信息
        continue;
      }

      unsigned int i = 0;
      char *q = msg;
      char *p;
      bool sendSuccess = false;
      char reason = 0;

      p = strsep(&q, ",");
      if (p && 0 == strcmp(p, "BDFKI")) {
        while (i < 5){
          p = strsep(&q, ",");
          if(NULL == p) break;
          if (i == 0){  // 指令发射时间（时分秒）
//            Serial.println(String("time:") + p);
          } else if (i == 1) {
//            Serial.println(String("type:") + p);
          } else if (i == 2) {
//            Serial.println(String("success:") + p);
            if (p[0] == 'Y') {
              sendSuccess = true;
            } else {
              sendSuccess = false;
            }
          } else if (i == 3) {
//            Serial.println(String("reason:") + p);
            reason = p[0];
          } else if (i == 4) {
//            Serial.println(String("time remaining:") + p);
          }
          i++;
        }

        if (sendSuccess && reason == '0') {
          result = 0;
        } else {
          Serial.println(String("Send Failed! reason is:"));
          switch (reason) {
            case '1': // 频率未到
              result = 6;
              break;
            case '2': // 发射抑制
              result = 7;
              break;
            case '3': // 发射静默
              result = 8;
              break;
            case '4': // 功率未锁定
              result = 9;
              break;
            case '5': // 未检测到IC模块信息
              result = 10;
              break;
            case '6': // 权限不足
              result = 11;
              break;
          }
        }
//        Serial.println(recvMsg);
        break;
      }
    }

    if (millis() - overTime >= 2000) {
      result = 12;
      break;
    }
  }
  return result;
}

// 校验码生成
char* getCheck(const char *str, int len) {
  static char checkcode[3];
  unsigned char c = str[0];
  for (int i = 1; i < len; i++) {
    c = str[i] ^ c;
  }
  sprintf(checkcode, "%02X", c);
  return checkcode;
}

// 英文数字符号字符串ASCII转GB2312全角宽字符（不包含中文）
char* getWCharString(char* str) {
  static char wcharStr[200] = {0};
  int len = strlen(str);
  int i = 0;
  for (; i < len; i++){
    wcharStr[i*2] = 0xA3;
    wcharStr[i*2+1] = 0x80 + str[i];
  }
  wcharStr[i*2] = 0;
  char* p = (char*)wcharStr;
  return p;
}

// 整型数字转GB2312全角宽字符串
char* getWCharString(int num){
  char str[33];
  sprintf(str, "%d", num);
  char* p = getWCharString(str);
  return p;
}

// 浮点数字转GB2312全角宽字符串
char* getWCharString(float num, int declen){
  char str[33];
  char sprintfPara[5] = {'%', '.', 0x30+declen, 'f', 0};
  sprintf(str, sprintfPara, num);
  char* p = getWCharString(str);
  return p;
}

// 错误原因打印。将错误代码编号解析成原因文字，通过Seriial串口打印出来
// 0-正常；1-无卡；2-ICP超时；3-无时间；4-无信号；5-PWI超时；6-频率未到；7-发射抑制；8-发射静默；9-功率未锁定；10-未检测到IC模块信息；11-权限不足； 12-FKI超时
void showError(int result) {
  switch(result) {
    case 1:
      Serial.println("No ID Info");   // 无北斗卡
      break;
    case 2:
      Serial.println("ICP Timeout");  // ICP消息超时
      break;
    case 3:
      Serial.println("No TIME");      // 未收到时间数据，不具备发送消息条件
      break;
    case 4:
      Serial.println("No Signal");    // 无波束信号
      break;
    case 5:
      Serial.println("PWI Timeout");  // PWI消息超时
      break;
    case 6:
      Serial.println("Frequency not reached");  // 频率未到
      break;
    case 7:
      Serial.println("Emission suppression");   // 发射抑制
      break;
    case 8:
      Serial.println("Launch Silence");         // 发射静默
      break;
    case 9:
      Serial.println("Power not locked");       // 功率未锁定
      break;
    case 10:
      Serial.println("IC module information not detected");  // 未检测到IC模块信息
      break;
    case 11:
      Serial.println("Insufficient permissions");  // 权限不足
      break;
    case 12:
      Serial.println("FKI Timeout");  // FKI超时
      break;
    default:
      Serial.println("Other Reason");  // 其他原因
      break;
  }
}

#endif
