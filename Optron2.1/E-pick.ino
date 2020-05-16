/*E-pick peak detection
 * 
 * 
 * Author:  Juwan Foreman
 * Date:  2/23/2020
 * 
 * Description: The OPRTON 2.2 has several sets of code, each specific to the sensor.
 * The E-pick is a circuit designed around the concept of voltage division. Here I am 
 * reading values through the pin A3.
 * 
 */

#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif

int val, cur = 0;
int temp = 0;
int next = 1;
int prev = -1;
int count = 7;
int avg = 0;
int randnum = 0;
int Peak[5];
int Trough[5];
int prevInput = 0;
int countUp = 0;
int peak = 0;
int trough = 0;



/*Peak/Trough detection
 * This will perform the peak and trough detection.
 * I have defined the high and low thresholds above in bit values as the analogRead function
 * returns a 10 bit value. This results in a 3.2mV increase per bit. 
 * 
 * 
 */
 
void readPick() {
  // put your main code here, to run repeatedly:
  count = 0;
  randnum = random(-100,100);
  if(randnum > 20){
    #ifdef OUTPUT_READABLE
      Serial.println("High end broken!");
      Serial.println(randnum);
    #endif
    PeakHandler(randnum);
    
    //Old implementation
    /*if(analogRead(Pin) > HighThreshold){
    val = analogRead(Pin);
    while(analogRead(Pin) > HighThreshold){
        PeakHandler(val);
        val = analogRead(Pin);
    }*/
    //delay(1000)
  }
  else if(randnum < -20){
    #ifdef OUTPUT_READABLE
      Serial.println("Low end broken!");
      Serial.println(randnum);
    #endif
    TroughHandler(randnum);
    
    //Old implementation
    /*else if(analogRead(Pin) < LowThreshold){
    val = analogRead(Pin);
    while(analogRead(Pin) < LowThreshold){
          TroughHandler(val);
          val = analogRead(Pin);
    }*/
    //delay(1000);
  }
  else{
      #ifdef OUTPUT_READABLE
        Serial.println("Threshold intact!");
        Serial.println(randnum);
      #endif
  }
}

/*PeakHandler
 * inputs: analog read integer
 * Outputs: none
 * Description: This function takes the analog value and fills half of an array with initial values.
 * It then checks for a peak value and when detected the array is filled with the next 7 values.
 * The function then outputs the array in reverse on the serial monitor. The peak value is also saved to a global variable.
 * 
 */
void  PeakHandler(int input){
  //New implementation
  while(input > 20){
    avg = 0;
    #ifdef OUTPUT_READABLE
      Serial.println(input);
    #endif
    if(count < 5){ /*This count variable is used to count the number of integers that exist within the Peak array. This value is used to calculate the average of said array.*/
      count = count + 1;
    }
    temp = Peak[0];
    /*Array shifter*/
    /*This for loop shifts each element in the array right by one.*/
    for(int i = 0; i < 4; i++){
      cur = temp;
      next = Peak[i+1];
      temp = next;
      Peak[i+1] = cur;
      avg = avg + cur;
    }
    Peak[0] = input;
    avg = avg + input;
    avg = avg/count;
    #ifdef OUTPUT_READABLE
      Serial.println("Average");
      Serial.println(avg);
    #endif
    if(input >= avg){
      peak = input;  
    }
    else {
      break;
    }
    input = random(-100,100);
  }
  /*Clear array loop*/
  for(int i = 0; i < 5; i++){
    Peak[i] = 0;
  }
}
  //Old implementation
   /*if(count > 0){
     // Serial.println("2");
     Peak[cur] = input;
     prev = cur;
     cur = next;
     next++;
     prevInput = input;
     count--;
     Serial.println(Peak[prev]);
  }
  if(count == 0 && prevInput > input && countUp < 5){
    Serial.println("peak reached!!");
     Peak[cur] = prevInput;
     Peak[next] = input;
     prevInput = input;
     countUp++;
     prev = cur;
     cur = next;
     next++;
     if(countUp == 1){
      peak = input;
     }
  }
  if(countUp == 5){
    count = 15;
    while(count){
      Serial.println(Peak[count]);
      Peak[count] = 0;
      count--;
    }
    countUp = 0;
    cur = 0;
    prev = -1;
    next = 1;
    delay(5000);
  }*/


/*TroughHandler
 * inputs: analog read integer
 * Outputs: none
 * Description: This function takes the analog value and fills half of an array with initial values.
 * It then checks for a trough value and when detected the array is filled with the next 7 values.
 * The function then outputs the array in reverse on the serial monitor. The trough value is also saved to a global variable.
 * 
 */
void TroughHandler(int input){
//New implementation
  while(input < -20){
    avg = 0;
    #ifdef OUTPUT_READABLE
      Serial.println(input);
    #endif    
    if(count < 5){/*This count variable is used to count the number of integers that exist within the Peak array. This value is used to calculate the average of said array.*/
      count = count + 1;
    }
    temp = Peak[0];
     /*Array shifter*/
    /*This for loop shifts each element in the array right by one.*/
    for(int i = 0; i < 4; i++){
      cur = temp;
      next = Trough[i+1];
      temp = next;
      Trough[i+1] = cur;
      avg = avg + cur;
    }
    Trough[0] = input;
    avg = avg + input;
    avg = avg/count;
    #ifdef OUTPUT_READABLE
      Serial.println("Average");
      Serial.println(avg);
    #endif
    if(input >= avg){
      trough = input;  
    }
    else {
      break;
    }
    input = random(-100,100);
  }
   /*Clear array loop*/
  for(int i = 0; i < 5; i++){
    Peak[i] = 0;
  }
}
//Old implementation
/*void  TroughHandler( int input){
  if(count > 0){
     Serial.println("2");
     Trough[cur] = input;
     prev = cur;
     cur = next;
     next++;
     prevInput = input;
     count--;
     Serial.println(Trough[prev]);
  }
  if(count == 0 && prevInput < input && countUp < 5){
    Serial.println("trough reached!!");
     Trough[cur] = prevInput;
     Trough[next] = input;
     prevInput = input;
     countUp++;
     if(countUp == 1){
        trough = input;
     }
  }
  if(countUp == 5){
    count = 15;
    while(count){
      Serial.println(Trough[count]);
      Trough[count] = 0;
      count--;
    }
    countUp = 0;
    cur = 0;
    prev = -1;
    next = 1;
    delay(5000);
  }
}*/
