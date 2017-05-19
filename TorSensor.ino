    #include <SPI.h>
    #include "nRF24L01.h"
    #include "RF24.h"
    #include "printf.h"
    
    // many staff you can config:
    #define SensorUniqeID 3                                   // !!! DO NOT USE 0 AS UNIQE ID it's used to present dead sensors !!!
    #define sensorTableSize 25                                // Balance between the memory using to remember friends and memory using to run your program.
    #define broadCastAdr 0xA8A8E1FC00LL
    #define privateAdr (broadCastAdr + SensorUniqeID)
    #define friendLestSeenTimeOut 300                          // Timeout for giveup on dead sensor.
    
    
    //const  byte sensorTableSize = 25;           // Balance between the memory using to remember friends and memory using to run your program.
    //const  long broadCastAdr = 0xA8A8E1FC00LL ;
    //const  byte SensorUniqeID = 1;              // !!! DO NOT USE 0 AS UNIQE ID it's used to present dead sensors !!!
    //const  byte friendLestSeenTimeOut = 30;     // Timeout for giveup on dead sensor.
    const int ldrPin = A0;  //the number of the LDR pin
    // flags:
    const byte ScreamFlag = B1;
    const byte MassageFlag = B10;
    
    /*
    * This struct peresent a row in the friends table (SensorTable).
    */
    typedef struct sensor {
      byte friendS;
      byte neighbor;
      byte distance;
      byte lastSeen;
    }Sensor;
    
    /* 
    * This struct hold "Scream" massage.
    * This is the way of sensor to say to others "im alive" if he know more sensors he will tell all who can hear him.
    */
    typedef struct screamMassage {
      byte me;
      byte flags;
      byte friends[15];
      byte distances[15];
    } ScreamMassage;
    
    typedef struct massage {
      byte me;
      byte flags;
      byte to;
      byte msg[29];
    } Massage;
    
    byte MaxCount = 0;
    unsigned long time;
    Sensor sensorTable[sensorTableSize];
    
    RF24 radio(9, 10);
    
    void setup() {
      int i;
      //Initialize Sensor Table
      for ( i = 0 ; i < sensorTableSize; i++) {
        sensorTable[i].friendS = B0;
        sensorTable[i].neighbor = B0;
        sensorTable[i].distance = B0;
        sensorTable[i].lastSeen = B0;
      }
      //Initialize Serial
      Serial.begin(57600);
      printf_begin();
    
      //Initialize RF24
      radio.begin();
     
      radio.setChannel(76);
      radio.openReadingPipe(1, broadCastAdr);
      radio.openReadingPipe(2, broadCastAdr + SensorUniqeID);
      radio.openWritingPipe(broadCastAdr);
      radio.setAutoAck(false);
      //radio.setAutoAck(1,false);
      //radio.setAutoAck(2,true);
      radio.setRetries (15, 1);
      radio.startListening();
      radio.printDetails();
    
      //Button configuration
      pinMode(2, INPUT);
    
      printf("init end!");
      
      //Interrupt  
      attachInterrupt(0, networkCorn, FALLING);    
    }
    
    void  loop() {
      unsigned long time = millis();
      unsigned long timeLs = millis();
      unsigned long timeToBroadcast = millis();
      char str[] = "hila_idan" ;
      scream();
      while (true){
        //networkCorn();
        
        if(time+2000 < millis()){
          printNebList();
         printf("");

          if( millis() - timeToBroadcast > 10000){
             scream();
             timeToBroadcast = millis();
          }
         
          time = millis();
          //sendPrivateMsg(str, 2);
           printf("");
         // sendSensorData();
          
        }
        if(timeLs+1000 < millis()){
           TimeOutCheck();
          timeLs = millis();
        }
        
      }
    }
    
    
    //void sendBorodCastMsg(struct screamMassage sm){}
    void sendBorodCastMsg(const void * payload , const uint8_t len){
          radio.stopListening();
          radio.setRetries (15, 1);
          radio.openWritingPipe(broadCastAdr);
          radio.write(payload, len);
          printf("Sent broadCast msg \n");
          // Now, resume listening so we catch the next packets.
          radio.startListening();
    }
    void sendPrivateMsg(const void * payload , const byte to){
      int i;
      Massage newMsg;
      newMsg.to = to;
      newMsg.me = SensorUniqeID;
      newMsg.flags = MassageFlag;
      byte * payarr = (byte *) payload;
      for(i = 0; i<29; i++){
        newMsg.msg[i] =  payarr[i];
      }
      sendPrivateMsg(&newMsg);
    }
      
    void sendPrivateMsg(struct massage * msg){
     for (int i = 0; i < MaxCount; i++) {
        if(msg -> to == sensorTable[i].friendS){
          radio.stopListening();
          radio.openWritingPipe(broadCastAdr + sensorTable[i].neighbor);
          radio.setRetries (15, 15);
          radio.write(msg, 32);
          printf("Sent private msg to %d \n",msg -> to);
          radio.startListening();
          break;
        }
      }
    }


     void sendSensorData(){
      int ldr = analogRead(ldrPin);
      char buffer[29];
      itoa(ldr, buffer , 10);
      printf("sending..\n");
      sendPrivateMsg(buffer, 1);  
    }
    
    void scream(){
      ScreamMassage sm;
      int count = 0;
      sm.me = SensorUniqeID;
      sm.flags = ScreamFlag;
      for (int i = 0; i < MaxCount; i++) {
        if (count >= 15) break;
        if (sensorTable[i].friendS != B0){
          sm.friends[count] = sensorTable[i].friendS;
          sm.distances[count] = sensorTable[i].distance;
          count++;
        }
      }
      for (; count < 15; count++) sm.friends[count] = B0;
      sendBorodCastMsg(&sm,32);
    }
    
    /**
    function that read from RF24 buffers and take care about new messages and need to be called permanently.
    **/
    void networkCorn() {
      printf("corn!\n");
      byte i;
      ScreamMassage msgBuff;
      if (radio.available()) {
        printf("got msg ");
        radio.read( &msgBuff, 32 );
        if (msgBuff.flags == ScreamFlag) {
          printf("and its a scream \n");
          addnewfriend(msgBuff.me, msgBuff.me, 1);
          for (i=0;i<15;i++){
           if(msgBuff.friends[i] == 0) break; //end of friends
           addnewfriend(msgBuff.friends[i], msgBuff.me, msgBuff.distances[i]+1); 
          }
        }else if (msgBuff.flags == MassageFlag){
          printf("and its a massage \n");
          massageHandler((struct massage*)&msgBuff);

        }
      //printf("Got payload %d...\n", msgBuff);
      }
    }
    
    void massageHandler(struct massage * msg){
      //char * buff  = (char*)msg->msg;
      if(msg -> to == SensorUniqeID){
            
            printf("and its for me... (%s)\n",(char*)msg->msg);
          }else{
            printf("and its for %d...\n", msg -> to);
            sendPrivateMsg(msg);
          }
    }
    
    /**
    This function increase the lastSeen counter in SensorTable from unseens Sensors
    **/
    void TimeOutCheck() {
      for (int i = 0; i < MaxCount; i++){
        sensorTable[i].lastSeen++;
        if( sensorTable[i].lastSeen >= friendLestSeenTimeOut) {
          sensorTable[i].friendS = B0;
          sensorTable[i].lastSeen = 0;
      }
    }
    }
    void printNebList(){
      printf("list of friends: (maxcount:%d) \n", MaxCount);
      for (int i = 0; i < MaxCount; i++) {
        if (sensorTable[i].friendS != B0) printf("[fr:%u neb:%u dist:%u ls:%u]\n",
                (unsigned int)sensorTable[i].friendS,
                (unsigned int)sensorTable[i].neighbor,
                (unsigned int)sensorTable[i].distance,
                (unsigned int)sensorTable[i].lastSeen);
      }
    }
    
    boolean addnewfriend(byte friendS, byte neighbor, byte distance) {
      if(friendS == SensorUniqeID) return false;
      for (int i = 0; i < MaxCount; i++) {
        if(friendS == sensorTable[i].friendS){      //friend already in my table
          if(distance > sensorTable[i].distance) return false;  // change noting - beacause i allready have shorter path.
          if(sensorTable[i].neighbor == neighbor && sensorTable[i].distance == distance){ // if its the same path to this friend- just update the last seen to 0.
            sensorTable[i].lastSeen = 0;
            return false;
          }
          if(distance < sensorTable[i].distance) sensorTable[i].friendS = B0; // remove the i friend from my table beacause new shortest path found.
          if(sensorTable[i].lastSeen > friendLestSeenTimeOut) sensorTable[i].friendS = B0; // remove the i friend from my table beacause lastseen>30.
        }
      }
        for (int i = 0; i < MaxCount; i++){  // if friend removed(timeoutcheck) ,we replace his line in my table with the new friend. 
            if(sensorTable[i].friendS == 0){
              sensorTable[i].distance = distance;
              sensorTable[i].neighbor = neighbor;
              sensorTable[i].friendS = friendS;
              sensorTable[i].lastSeen = 0;
              return true;
            }
        }
        if (MaxCount < sensorTableSize){     // add new friend and update the maxcount.
              sensorTable[MaxCount].distance = distance;
              sensorTable[MaxCount].neighbor = neighbor;
              sensorTable[MaxCount].friendS = friendS;
              sensorTable[MaxCount].lastSeen = 0;
              MaxCount++;
              return true;
        }
    }

