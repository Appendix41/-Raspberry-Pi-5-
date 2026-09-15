/*
  接线方式：
    模块    ARDUINO
    VCC----5V
    GND----GND
    RXD ---TX [不接]
    TXD---->RX   [如果是UNO就接0，如果是其它的多串口板子，比如mega2560,leonardo等，就接RX1]

*/


//如果是多串口板子比如mega2560，micro板子可以串口看数据
#if defined(UBRR1H)
  #define DebugSerial Serial
  #define SensorSerial Serial1
  #define DEBUGSendON
#else //如果是UNO板子只有一个串口，要这样定义
  #define DebugSerial Serial
  #define SensorSerial Serial
  #define DEBUGSendON 
#endif

#define Buf_Max 80
char Rec_Buf[Buf_Max] = {}; //串口数据缓存
int RxCount = 0;

bool bufferComplete = false;
int p_pressure[19]={0};
#define samplingtime 1000//采集模块采样频率，需修改

void Set_handler(void); //设置处理函数;

void setup()
{
  SensorSerial.begin(9600);
  DebugSerial.begin(9600);
}

void loop()
{
  static unsigned long samplingTime = millis();
  int i=0;

  while (DebugSerial.available())
  {
    // get the new byte:
    Rec_Buf[RxCount++] = (char)SensorSerial.read();
    if (RxCount >= Buf_Max)
    {
      RxCount = 0;
    }
    bufferComplete = true;
  }

    if(millis()-samplingTime > samplingtime)
  {
      Set_handler();
      for(i=1;i<19;i++)
     {
       DebugSerial.print('[');
       DebugSerial.print(i,DEC);
       DebugSerial.print("] ");
       DebugSerial.print(p_pressure[i-1]);
     }
      DebugSerial.println();
      samplingTime=millis();
  }
    delay(50);
}


void CLR_Buf(void) // 串口缓存清理
{
  memset(Rec_Buf, 0, Buf_Max); //清空
  RxCount = 0;
}
void Set_handler(void) //设置处理函数
{
        p_pressure[0]=Rec_Buf[2]*256+Rec_Buf[3];
        p_pressure[1]=Rec_Buf[4]*256+Rec_Buf[5];
        p_pressure[2]=Rec_Buf[6]*256+Rec_Buf[7];
        p_pressure[3]=Rec_Buf[8]*256+Rec_Buf[9];
        p_pressure[4]=Rec_Buf[10]*256+Rec_Buf[11];
        p_pressure[5]=Rec_Buf[12]*256+Rec_Buf[13];
        p_pressure[6]=Rec_Buf[14]*256+Rec_Buf[15];
        p_pressure[7]=Rec_Buf[16]*256+Rec_Buf[17];
        p_pressure[8]=Rec_Buf[18]*256+Rec_Buf[19]; 
        p_pressure[9]=Rec_Buf[20]*256+Rec_Buf[21];
        p_pressure[10]=Rec_Buf[22]*256+Rec_Buf[23];
        p_pressure[11]=Rec_Buf[24]*256+Rec_Buf[25];
        p_pressure[12]=Rec_Buf[26]*256+Rec_Buf[27];
        p_pressure[13]=Rec_Buf[28]*256+Rec_Buf[29];
        p_pressure[14]=Rec_Buf[30]*256+Rec_Buf[31];
        p_pressure[15]=Rec_Buf[32]*256+Rec_Buf[33];   
        p_pressure[16]=Rec_Buf[34]*256+Rec_Buf[35];
        p_pressure[17]=Rec_Buf[36]*256+Rec_Buf[37]; 
        CLR_Buf();
}
