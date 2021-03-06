#include <max6675.h>
#define board mega2560
#define ActiveLedPin 13
#define control_pin 12 
// 20180920 fix Runjob using Showcolor' problems , instead of using 
//  ChangeBulbColor(RedValue,GreenValue,BlueValue,LightValue) ;
// ThermoCouple
  //  #define thermo_vcc_pin     13 
    #define thermo_sck_pin    9 
    #define thermo_cs_pin    10 
    #define thermo_so_pin   11
MAX6675 thermocouple(thermo_sck_pin, thermo_cs_pin, thermo_so_pin);
//used to control SSR

int TempValue = 0 ;
int TmpValue = 0 ;

#define ThermCount 8
#define WAITSEC 6
#define dutycycle 250
#define PWMLevel 16     // pwm max level
int PWMControl = 15 ;   // pwn control level setting

//-------------------------
#include <EEPROM.h>
#define eDataAddress 100
int ParaCount = 0 ;
int PreHotParameter[2] = {20,170};
//int PreHotParameter[2] = {180,170};
int HotParameter[10][2] = { 
                            {180 ,150 },{360 ,175 },
                            {660 ,210 },{0 ,0 }, 
                            {0 ,0 },{0 ,0 }, 
                            {0 ,0 },{0 ,0 }, 
                            {0 ,0 },{0 ,0 }
};
//--------------------
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//----------------------
//----------------------
#include <Keypad.h>
char KeyPadChar[] = {'0','1','2','3','4','5','6','7','8','9','*','#','A','B','L','R','U','D','E','X'} ;
char KeyPadNum[] = {'0','1','2','3','4','5','6','7','8','9','L' ,'E','X'} ;
char KeyPadPage[] = {'U','D','E','X'} ;
char KeyPadCmd[] = {'A','B','#','*'} ;
char KeyPadYesNo[] = {'0','1'} ;
const byte ROWS = 4; //four rows
const byte COLS = 5; //three columns
char keys[ROWS][COLS] = 
  {
    {'L','7','4','1','A'},
    {'0','8','5','2','B'},
    {'R','9','6','3','#'},
    {'X','E','D','U','*'}
  };
  
byte rowPins[ROWS] = {22, 24, 26, 28 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = {30 ,32,34 ,36 ,38}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//--------------------
boolean InputStatus = false ;
boolean WorkStatus = false ;
unsigned stepnum = 1 ;
unsigned posnum = 0 ;
char inptxt[3]  ;
int pos1[3][2] = {{1,13},{1,14},{1,15}} ;
int pos2[3][2] = {{2,13},{2,14},{2,15}} ;
char pchar ;
int rulenum = 1 ;
//--------------------

//---------------
//------------------
#define PLEVEL 8
#define WAITSEC 6

void setup()
{
  initAll() ;
   ShowStartUP() ;
  delay(1000) ;
  ShowScreen() ;
}

void loop()
{
     Dialog("F1 PreHot,'F2 Run") ;
  
  pchar = InstantKeyInput() ;
  Serial.print("[") ;
  Serial.print(pchar) ;
   Serial.print("]\n") ;  
 if ( CheckKeyPadCMD(pchar)=='A' )
  {
    Serial.println("Run Pre Hot") ;
    Dialog("Run Pre Hot") ;
     HotRun(PreHotParameter[0],PreHotParameter[1] , "Pre Hot Activcated") ;
       ShowScreen() ;
    Dialog("F1 PreHot,'F2 Run") ;
  }   // end of ( CheckKeyPadStart(InstantKeyInput()) )

  if ( CheckKeyPadCMD(pchar)=='B' )
  {
    Serial.println("Run Bean Roaster") ;
    Dialog("Run Bean Roaster") ;
     RunHot() ;
       ShowScreen() ;
    Dialog("F1 PreHot,'F2 Run") ;    
  }   // end of ( CheckKeyPadStart(InstantKeyInput()) )
 if ( CheckKeyPadCMD(pchar)=='#' )
  {
    Serial.println("Set Pre Hot") ;
    Dialog("Set Pre Hot") ;
      if (SetPreHot())
            savePreHotParameter() ;
         HideInputCursor() ;     
        ShowScreen() ;
        Dialog("F1 PreHot,'F2 Run") ;
  }   // end of ( CheckKeyPadStart(InstantKeyInput()) )
 if ( CheckKeyPadCMD(pchar)=='*' )
  {
    Serial.println("Set Temperature & Time") ;
    HotMenu() ;
      ShowScreen() ;
    Dialog("F1 PreHot,'F2 Run") ;
      //initinptxt() ;
  }   // end of ( CheckKeyPadStart(InstantKeyInput()) )








}   // end of loop
//---------------
void RunHot()
{
  for (int i=0 ; i <10;i++)
  {
      if (HotParameter[i][0] >0 )
      {
          HotRun(HotParameter[i][0],HotParameter[i][1] , "Hot Activcated:("+String(i+1)+")") ;
      }     // is work if (HotParameter[i][0] >0 )
    if (InstantKeyInput() == 'E')
    {
          TutnOff() ;   // too hot and turn off
          return ;          
    }
  }   // end of for (int i=0 ; i <10;i++)
}   // end of void RunHot()

void HotRun(int during, int hot, String st)
{
  
  TmpValue = 0 ;
  TempValue = 0;
  int pcount = 0 ;
  int pwmcount  = 0 ;
  long strtime = millis() ;
   displayLcd(1,st) ;
  while (((millis() -strtime)/1000)  <during)
  {
  displayLcd(2,"Temperature:"+String(TempValue)+"/"+String(hot)) ;
  displayLcd(3,"Time:"+String(during - (int)((millis() -strtime)/1000) ) +"/"+String(during)) ; ;
    if (InstantKeyInput() == 'E')
    {
          TutnOff() ;   // too hot and turn off
          return ;          
    }

      
    if (pcount < ThermCount)
      {
         TmpValue = TmpValue + thermocouple.readCelsius();
          pcount ++ ;
          Serial.print("Hot:(");
          Serial.print(pcount);
          Serial.print("/");          
          Serial.print(thermocouple.readCelsius());
          Serial.print("/");          
          Serial.print(TmpValue);   
          Serial.print(")\n");   
      }
      else
      {
        pcount = 0 ;
        TempValue = (TmpValue /ThermCount) ;
        TmpValue = 0 ;
     //   displayLcd(2,"Now Temperature :"+String(TempValue)) ;
     //   displayLcd(3,"Reserved Time:"+String(during - ((millis() -strtime)/1000))) ; ;
          //control PWN HOT
       }
      
        if  (TempValue <=hot)   // inder hot
          {
              PWMHotControl(pwmcount) ;    
              //   open hot supplier
          }
          else
          {
              TutnOff() ;   // too hot and turn off
          }
      pwmcount ++ ;       // pwm control variable
      if (pwmcount >=PWMLevel)    // CHECK pwm level to turn on
        {
          pwmcount = 0 ;    //set pwmcount to ZERO
        }
      delay(dutycycle) ;      // delay for dutycycle
  }   // end of while ((millis() -strtime) <(during * 1000))
  TutnOff() ;   // too hot and turn off
} // end of void HotRun(int during, int hot , String st)
    

  
void PWMHotControl(int hotccont )
{
  
  if (hotccont < PWMControl)
  {
     TutnOn() ;
  }
  else
  {
     TutnOff() ;
  }
  
}

boolean HotMenu()
{
    Dialog("UP/Down Enter:Edit") ;
    pchar =1  ;
    while ( true)
    {
        if (pchar > 0 ) 
        {
            Serial.print("Menu:(") ;
            Serial.print(pchar) ;
            Serial.print(")\n") ;
            Dialog("UP/Down Enter:Edit") ;
            if (CheckPageView(pchar) =='U')
                rulenum -- ;
             if (CheckPageView(pchar) =='D')
                rulenum ++ ;
             if (rulenum >10)
                  rulenum = 1 ;
             if (rulenum < 1)
                  rulenum = 10 ;
            PageViewScreen(rulenum,HotParameter[rulenum-1][0], HotParameter[rulenum-1][1], "View Control Rules" ) ;

            }
      if (CheckPageView(pchar) =='X') 
      {
          if (SetHot(rulenum))
            {
                saveHotParameter(rulenum) ;
            }
        HideInputCursor() ;    
            PageViewScreen(rulenum,HotParameter[rulenum-1][0], HotParameter[rulenum-1][1], "View Control Rules" ) ;
        
      }
          if (CheckPageView(pchar) =='E')
              {
                return true ;
              }      
        pchar = InstantKeyInput() ; 
      }
}   // end of function

boolean SetHot(int rl)
{
    Dialog("ESC Abort, Enter OK") ;
      int temp1 ,temp2 ;
      temp1 = HotParameter[rl-1][0] ;
      temp2 = HotParameter[rl-1][1] ;
      
     OperatingScreen(0, temp1 , temp2 , "Set Hot & Time") ;
          Serial.println("Wait for input for Controling Temperature time") ;
          Dialog("Input Hot Time") ;
        posnum = 0 ;
        Setinptxt(temp1) ;
        while (posnum < 3)  // input six number for schedule
          {
              ShowInputCursor(pos1[posnum][0],pos1[posnum][1])   ;
              pchar = CheckInputfromKeyPad(KeyInput()) ;
              // check input backward
              if (pchar == 'L')
              {
                  if (posnum > 0)
                    {
                      posnum -- ;
                    }
                  continue  ;
              }   // end of pchar == '*' check
              if (pchar == 'E')
              {
                    return false ;
              }          
              if (pchar == 'X')
              {
                      temp1 = ((int)inptxt[0]-48)*100+((int)inptxt[1]-48)*10+((int)inptxt[1]-48) ;
                       Serial.println(temp1) ;
                       Serial.println(temp2) ;
                      
                   pchar = CheckYesNo(KeyYesNo()) ;
                    if (pchar == '0')
                      {
                          return false ;
                      }
                      else
                      {
                              
                          HotParameter[rl-1][0] =temp1  ;
                          HotParameter[rl-1][1] = temp2 ;
                          return true ;
                      }                       
              }   
              // now check real input
            if (pchar != 0)
            {
                inptxt[posnum] = pchar ;
                ShowInputChar(pos1[posnum][0],pos1[posnum][1], inptxt[posnum])  ;
                posnum ++ ;
                
            }

          }   // end of while (posnum < 3) for Check  six char input
        temp1 = ((int)inptxt[0]-48)*100+((int)inptxt[1]-48)*10+((int)inptxt[1]-48) ;
        Serial.println(temp1) ;
        
           Serial.println("Wait for input for Control Temperature") ;
          Dialog("Input Temperature") ;
        posnum = 0 ;
        Setinptxt(temp2) ;
        while (posnum < 3)  // input six number for schedule
          {
              ShowInputCursor(pos2[posnum][0],pos2[posnum][1])   ;
              pchar = CheckInputfromKeyPad(KeyInput()) ;
              // check input backward
              if (pchar == 'L')
              {
                  if (posnum > 0)
                    {
                      posnum -- ;
                    }
                  continue  ;
              }   // end of pchar == '*' check
              if (pchar == 'E')
              {
                    return false ;
              }    
               if (pchar == 'X')
              {
                      temp2 = ((int)inptxt[0]-48)*100+((int)inptxt[1]-48)*10+((int)inptxt[2]-48) ;
                      Serial.println(temp2) ;
                      
                   pchar = CheckYesNo(KeyYesNo()) ;
                    if (pchar == '0')
                      {
                          return false ;
                      }
                      else
                      {
                              
                          HotParameter[rl-1][0] =temp1  ;
                          HotParameter[rl-1][1] = temp2 ;
                          return true ;
                      }                       
              }              
              // now check real input
            if (pchar != 0)
            {
                inptxt[posnum] = pchar ;
                ShowInputChar(pos2[posnum][0],pos2[posnum][1], inptxt[posnum])  ;
                posnum ++ ;
                
            }

          }   // end of while (posnum < 3) for Check  six char input
        temp2 = ((int)inptxt[0]-48)*100+((int)inptxt[1]-48)*10+((int)inptxt[2]-48) ;
        Serial.println(temp2) ;
        
     pchar = CheckYesNo(KeyYesNo()) ;
      if (pchar == '0')
        {
            return false ;
        }
        else
        {
                
            HotParameter[rl-1][0] =temp1  ;
            HotParameter[rl-1][1] = temp2 ;
            return true ;
        }
}   // end of function

boolean SetPreHot()
{
    Dialog("ESC Abort, Enter OK") ;
      int temp1 ,temp2 ;
      temp1 = PreHotParameter[0] ;
      temp2 = PreHotParameter[1] ;
      Serial.print("Prehot:(") ;
      Serial.print(temp1) ;
      Serial.print("/") ;
      Serial.print(temp2) ;
      Serial.print(")\n") ;
     OperatingScreen(0, temp1 , temp2 , "Set Pre Hot & Time") ;
          Serial.println("Wait for input for PreHot Time") ;
          Dialog("Input PreHot Time") ;
        posnum = 0 ;
        Setinptxt(temp1) ;
        while (posnum < 3)  // input six number for schedule
          {
              ShowInputCursor(pos1[posnum][0],pos1[posnum][1])   ;
              pchar = CheckInputfromKeyPad(KeyInput()) ;
              // check input backward
              if (pchar == 'L')
              {
                  if (posnum > 0)
                    {
                      posnum -- ;
                    }
                  continue  ;
              }   // end of pchar == '*' check
              if (pchar == 'E')
              {
                    return false ;
              }               // now check real input
              if (pchar == 'X')
              {
                temp1 = ((int)inptxt[0]-48)*100+((int)inptxt[1]-48)*10+((int)inptxt[2]-48) ;
                  Serial.println(temp2) ;
                  
               pchar = CheckYesNo(KeyYesNo()) ;
                if (pchar == '0')
                  {
                      return false ;
                  }
                  else
                  {
                          
                      PreHotParameter[0] = temp1  ;
                      PreHotParameter[1] = temp2 ;
                      return true ;
                  }
              }               // now check real input              
            if (pchar != 0)
            {
                inptxt[posnum] = pchar ;
                ShowInputChar(pos1[posnum][0],pos1[posnum][1], inptxt[posnum])  ;
                posnum ++ ;
                
            }

          }   // end of while (posnum < 3) for Check  six char input
        temp1 = ((int)inptxt[0]-48)*100+((int)inptxt[1]-48)*10+((int)inptxt[2]-48) ;
        Serial.println(temp1) ;
        
           Serial.println("Wait for input for prehot Temperature") ;
          Dialog("InputTemperature") ;
        posnum = 0 ;
        Setinptxt(PreHotParameter[1]) ;
        while (posnum < 3)  // input six number for schedule
          {
              ShowInputCursor(pos2[posnum][0],pos2[posnum][1])   ;
              pchar = CheckInputfromKeyPad(KeyInput()) ;
              // check input backward
              if (pchar == 'L')
              {
                  if (posnum > 0)
                    {
                      posnum -- ;
                    }
                  continue  ;
              }   // end of pchar == '*' check
              if (pchar == 'E')
              {
                    return false ;
              }               // now check real input
              if (pchar == 'X')
              {
                  Serial.println(temp1) ;
                 temp2 = ((int)inptxt[0]-48)*100+((int)inptxt[1]-48)*10+((int)inptxt[2]-48) ;
                Serial.println(temp2) ;
                 
               pchar = CheckYesNo(KeyYesNo()) ;
                if (pchar == '0')
                  {
                      return false ;
                  }
                  else
                  {
                          
                      PreHotParameter[0] = temp1  ;
                      PreHotParameter[1] = temp2 ;
                      return true ;
                  }
              }                   
            if (pchar != 0)
            {
                inptxt[posnum] = pchar ;
                ShowInputChar(pos2[posnum][0],pos2[posnum][1], inptxt[posnum])  ;
                posnum ++ ;
                
            }

          }   // end of while (posnum < 3) for Check  six char input
        temp2 = ((int)inptxt[0]-48)*100+((int)inptxt[1]-48)*10+((int)inptxt[2]-48) ;
        Serial.println(temp2) ;
        
     pchar = CheckYesNo(KeyYesNo()) ;
      if (pchar == '0')
        {
            return false ;
        }
        else
        {
                
            PreHotParameter[0] = temp1  ;
            PreHotParameter[1] = temp2 ;
            return true ;
        }
}   // end of function

void PageViewScreen(int st,int temp, int durtme, String Ti)
{
  lcd.clear() ;
   lcd.setCursor(0,0);
  lcd.print(Ti);
   lcd.setCursor(0,1);
  lcd.print("Rule:(") ;
    lcd.print(st) ;
  lcd.print(")") ;    
   lcd.setCursor(0,2);
   lcd.print("Time:") ;
    lcd.print(strzero(temp,3,10)) ;    
    lcd.print(" , Temp:") ;
    lcd.print(strzero(durtme,3,10)) ;  
    
}

void OperatingScreen(int st,int temp, int durtme, String Ti)
{
  lcd.clear() ;
   lcd.setCursor(0,0);
  lcd.print(Ti);
   lcd.setCursor(0,1);
  lcd.print("Stage:") ;
    lcd.print(st) ;
   lcd.setCursor(8,1);
  lcd.print("Time:") ;
    lcd.print(strzero(temp,3,10)) ;    
    lcd.setCursor(0,2);
  lcd.print("Temperature :") ;
    lcd.print(strzero(durtme,3,10)) ;  
    
}


//-------------
char CheckInputfromKeyPad(char kp)
{
    for(int i = 0 ; i <  sizeof(KeyPadNum) ; i++)
      {
          if  (KeyPadNum[i]  == kp) 
              return kp ;
      }
    return 0 ;
}





void ShowScreen()
{
  lcd.clear() ;
   lcd.setCursor(0,0);
  lcd.print("Coffee Bean Roaster");
}





void ShowStartUP()
{
  lcd.clear() ;
   lcd.setCursor(0,0);
  lcd.print("Coffee Bean Roaster");
   lcd.setCursor(0,1);
  lcd.print("Research by NCNU EEE") ;
   lcd.setCursor(0,2);
  lcd.print(" Dr.Yaw-Wen Kuo");
   lcd.setCursor(0,3);
  lcd.print(" Dr.YC Tsao");

  
}

void initAll()
{
  Serial.begin(9600) ;
   Serial1.begin(9600); //  
   pinMode(ActiveLedPin,OUTPUT) ;
   pinMode(control_pin,OUTPUT) ;
   digitalWrite(ActiveLedPin,LOW) ;
  digitalWrite(control_pin,LOW); 
   
  Serial.println("Program Start") ;
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.clear() ;

  ParaCount = EEPROM.read(eDataAddress);
  if (!(ParaCount >=0 && ParaCount <=99))
      {
        ParaCount = 0 ;
      }
 

}

void Dialog(String str)
{
    DialogClear() ;
   lcd.setCursor(0,3);                                                          
   lcd.print(str);  
}

void DialogClear()
{
     lcd.setCursor(0,3);                                                          
   lcd.print("                    ");  
}

//--------------
char CheckPageView(char kp)
{
    for(int i = 0 ; i <  sizeof(KeyPadPage) ; i++)
      {
          if  (KeyPadPage[i]  == kp) 
              return kp ;
      }
    return 0 ;
}

char CheckKeyPadCMD(char kp)
{
    for(int i = 0 ; i <  sizeof(KeyPadCmd) ; i++)
      {
          if  (KeyPadCmd[i]  == kp) 
              return kp ;
      }
    return 0 ;
}


char InstantKeyInput()
{
    char key ;
    long st= millis() ;
    while (1)
    {
      if ((millis() - st) > 300)
        break ;
      
      key = keypad.getKey();
      
       if (key == 0)
          {
            delay(20) ;
            // Serial.println("Wait.....") ;
          }
          else
          {
           //  Serial.println("Got Key") ;
             break ;
            // delay(3000) ;
          }
   }
   return  key ;
}

char KeyInput()
{
    char key ;
    while (true)
    {
      key = keypad.getKey();
        Serial.print("[") ;
       Serial.print(key,HEX) ;
        Serial.print("]\n") ;
         if (key == 0)
            continue ;
        if (CheckKeyPadChar(key) >0)
        {
          //  Serial.print("Bingo\n") ;
            return  key ;
        }   
   }
   return  key ;
}
char CheckKeyPadChar(char kp)
{
    for(int i = 0 ; i <  sizeof(KeyPadChar) ; i++)
      {
          if  (KeyPadChar[i]  == kp) 
              return kp ;
      }
    return 0 ;
}
void ShowInputChar(int y, int x, char ct)
{
          lcd.setCursor(x,y);
          lcd.print(ct) ;
  
}
void HideInputCursor()
{
          lcd.noBlink();
  
}

void ShowInputCursor(int y, int x)
{
          lcd.setCursor(x,y);
          lcd.blink() ;
  
}

void Setinptxt(int no)
{
    String tmp = strzero(no,3,10) ;
   for(int i=0 ; i <3; i++)
      inptxt[i] = (char)(tmp.substring(i,i).toInt()+48) ;
}
char KeyYesNo()
{
    char key ;
    Dialog("0=Abort,1=Proceed") ;
    while (true)
    {
      key = keypad.getKey();
      //  Serial.print("[") ;
      //  Serial.print(key,HEX) ;
      //  Serial.print("]\n") ;
         if (key == 0)
            continue ;
        if (CheckYesNo(key) >0)
        {
          //  Serial.print("Bingo\n") ;
            return  key ;
        }   
   }
   return  key ;
}
char CheckYesNo(char kp)
{
    for(int i = 0 ; i <  sizeof(KeyPadYesNo) ; i++)
      {
          if  (KeyPadChar[i]  == kp) 
              return kp ;
      }
    return 0 ;
}

void savePreHotParameter()
{
    EEPROM.write(eDataAddress , PreHotParameter[0] ) ;
    EEPROM.write((eDataAddress +5), PreHotParameter[1]  ) ;
}
void saveHotParameter(int rnum)
{
    EEPROM.write((eDataAddress+100 +(rnum -1) * 10), HotParameter[rnum -1][0] ) ;
    EEPROM.write((eDataAddress+100 +5+(rnum -1) * 10), HotParameter[rnum -1][1] ) ;
}

void RestoreParemeter()
{
  
}
void displayLcd(int ro, String st)
{
  // ro is n line( 1 is first
  // st is display message
    lcd.setCursor(0,ro-1);
    lcd.print("                    ") ;
    lcd.setCursor(0,ro-1);
    lcd.print(st) ;     
}



void TutnOn()
{
    digitalWrite(ActiveLedPin,HIGH) ;
    digitalWrite(control_pin,HIGH) ;
}
void TutnOff()
{
    digitalWrite(ActiveLedPin,LOW) ;
    digitalWrite(control_pin,LOW) ;    
}


