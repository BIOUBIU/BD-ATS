#include <RtcDS1302.h>
#include <ThreeWire.h>
#include <ArduinoSTL.h>
#include <string.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#include <map>
#include <queue>
#include "bdrd_esp32.h"
using namespace std;
ThreeWire myWire(4,5,2);
RtcDS1302<ThreeWire> Rtc(myWire); 
HardwareSerial BDR(2);
HardwareSerial RP(3);
SoftwareSerial BDP(21,22);
map<String,String> Tasks_Todo;
queue<String> Q;
int RP_Ready=0;
int WakeUp_Pin=23;

void WakeUp()
{
  digitalWrite(WakeUp_Pin,LOW);
  delay(50);
  digitalWrite(WakeUp_Pin,HIGH);
}

char* BD_GetTime()
{
  String temp=String(BDP.readStringUntil('\n'));
  temp=String(BDP.readStringUntil('\n'));
  String time=String(temp.substring(temp.indexOf(',')+1,temp.indexOf('\r'))+' '+temp.substring(0,temp.indexOf(',')+'\n'));
  return time;
}

String RTC_GetTime()
{
  RtcDateTime now=Rtc.GetDateTime();
  char dt[20];
  snprintf_P(dt,countof(dt),PSTR("%04u-%02u-%02u %02u:%02u:%02u"),now.Year(),now.Month(),now.Day(),now.Hour(),now.Minute(),now.Second());
  String time=String(dt);
  return time;
}

void serialEvent()
{
  if(RP.available())
  {
    char *temp="";
    RP.readBytesUntil(',',temp,4);
    String temp1=String(temp);
    if(temp1=="AFT\n")
    {
      RP.println(String("TCB,"+RTC_GetTime()));
    }else if (temp1=="RIO\n")
    {
      RP_Ready=0;
      BDT.println("Respery Pie has been asleep");
    }else
    {
      if(temp1=="CHT")
      {
        String temp2=RP.readStringUntil('\n');
        if(temp2==RTC_GetTime())
        {
          RP_Ready=1;
        }else
        {
          String temp3=String("TCB,"+RTC_GetTime());
          RP.println(temp3);
        }
      }
      if(temp1=="MSG")
      {
        String temp2=String("MSG,"+RP.readStringUntil('$'));
        BDT.println(temp2);
      }
    }
  }
}
void serialEvent1()
{
  if(BDR.available())
  {
    String temp1=BDR.readStringUntil('\n');
    if(temp1.startsWith("TSK"))
    {
      int length=temp1.length();
      RtcDateTime Task_Time_From=RtcDateTime(2008,08,08,20,00,00);
      Task_Time_From.InitWithDateTimeFormatString(F("YYYY-MM-DD hh:mm:ss"),temp1.substring(length-39,length-20));
      Task_Time_From-=180;
      char dt[20];
      sprintf_P(dt,countof(dt),PSTR("%04u-%02u-%02u %02u:%02u:%02u"),Task_Time_From.Year(),Task_Time_From.Month(),Task_Time_From.Day(),Task_Time_From.Hour(),Task_Time_From.Minute(),Task_Time_From.Second());
      String Time_Pre=String(dt);
      if(temp1.charAt(4)=='1')
      {
        String temp2=String("TSK,"+temp1.substring(4)+'\n');
      }else
      {
        int id5=-1,id3=-1;
        for(int i=1;i<=5;i++)
        {
          id5=temp1.indexOf(',',id5+1);
          if(i==3) id3=id5;
        }
        String temp2=String("TLE,"+temp1.substring(6,id5)+'\n');
        Q.push(temp2);
        temp2=String("TSK,"+temp1.substring(6,id3)+temp1.substring(id5)+'\n');
      }
      Tasks_Todo[Time_Pre]=temp2;
    }else
    {
      String temp2=String(temp1+'\n');
      Q.push(temp2);
    }
  }
}
void setup() {
  pinMode(WakeUp_Pin,OUTPUT);
  digitalWrite(WakeUp_Pin,HIGH);
  BDR.begin(115200,SERIAL_8N1,16,17);
  RP.begin(115200,SERIAL_8N1,26,27);
  BDP.begin(9600);
  BDP.listen();
  Rtc.Begin();
  RtcDateTime compileDateTime=RtcDateTime(2008,08,08,20,00,00);
  String BD_Time=BD_GetTime();
  compileDateTime.InitWithDateTimeFormatString(F("YYYY-MM-DD hh:mm:ss"),BD_Time);
  Rtc.SetDateTime(compileDateTime);
  String message=String("MSG,0,RTC_Time has been set at "+BD_Time+"(UTC)");
  BDT.println(message);
  deley(1000);
}

void loop() {
  if(!Q.empty())
  {
    if(RP_Ready)
    {
      RP.print(Q.front());
      Q.pop();
    }else
    {
      WakeUp();
    }
  }
  if(!Tasks_Todo.empty())
  {
    if(Tasks_Todo.begin()->first<=RTC_GetTime())
    {
      if(RP_Ready)
      {
        RP.print(Tasks_Todo.begin()->second);
        Tasks_Todo.erase(Tasks_Todo.begin());
      }else
      {
        WakeUp();
      }
    }
  }
}
