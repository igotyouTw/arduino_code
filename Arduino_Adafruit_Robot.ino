#include <AFMotor.h>
#include <Servo.h> 
#include <NewPing.h>

#define TRIG_PIN A4 //A4 超聲波pin 腳
#define ECHO_PIN A5 //A5 超聲波pin 腳
#define MAX_DISTANCE_POSSIBLE 1000  //最大超音波距離
#define MAX_SPEED 150 //  最高轉速(rpm)
#define MOTORS_CALIBRATION_OFFSET 0 //左右輪的校正參數
#define COLL_DIST 10 //危險距離  必須後退
#define TURN_DIST COLL_DIST+5  //警示距離  必須轉彎
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE_POSSIBLE); //使用newPing來使用超聲波  據說有比較優化超聲波模組

AF_DCMotor leftMotor(4, MOTOR12_8KHZ); //馬達宣告
AF_DCMotor rightMotor(3, MOTOR12_8KHZ);//馬達宣告 
Servo neckControllerServoMotor; //伺服馬達宣告

int pos = 0;    //伺服馬達當前的角度
  int maxDist = 0; //離障礙物最大距離
  int maxAngle = 0; //計算障礙物距離之後，推算轉彎方向的數直  
  int maxRight = 0; //右邊障礙物距離  
  int maxLeft = 0;//左邊障礙物距離   
  int maxFront = 0;//前邊障礙物距離  
int course = 0;  //程式碼沒用到
int curDist = 0; //當前超聲波測到的障礙物距離
String motorSet = ""; //記錄當前馬達該做甚麼動做的字串變數
int speedSet = 0; //當前馬達的轉速(rpm)



void setup() { 
 
 
 neckControllerServoMotor.attach(10);  //連結伺服馬達與腳位
  neckControllerServoMotor.write(90); //轉到90度  也就是正前方
  delay(2000);
  
  checkPath();  //判斷該往哪邊走的函式
  motorSet = "FORWARD"; 
  neckControllerServoMotor.write(90);
  moveForward();
  
}



void loop() {
  
  checkForward();  //判斷是否該往前走
  checkPath();   //判斷該往哪邊走的函式
  
  
  //moveForward();
}

//檢查各方向障礙物距離
//如果有危險距離:就先後退，在依照maxAngle的數值轉彎
//如果有警示距離:就依照當前角度  pos的位置進行判斷 ，往左/右邊看
//如果沒有以上情況，就去刷新各方向最遠障礙物的距離
void checkPath() {
  //歸零
  int curLeft = 0;
  int curFront = 0;
  int curRight = 0;
  int curDist = 0;
  //伺服馬達轉到最左邊
  neckControllerServoMotor.write(144);
  delay(120); 
  //每個定點方向  都偵測一次障礙物距離
  for(pos = 144; pos >= 36; pos-=18)
  {
    neckControllerServoMotor.write(pos);
    delay(90);
    checkForward(); 
    //抓距離
    curDist = readPing();
    //判斷是否有障礙物
    if (curDist <= COLL_DIST) {
      //後退  然後跳出
      checkCourse();
      break;
    }else if (curDist <= TURN_DIST) {
      //轉彎
      changePath();
    }
    //如果最遠距離變更大了，刷新距離  
    if (curDist > maxDist) {maxAngle = pos; maxDist=curDist;} 
    if (pos > 90 && curDist > curLeft) { curLeft = curDist;}
    if (pos == 90 && curDist > curFront) {curFront = curDist;}
    if (pos < 90 && curDist > curRight) {curRight = curDist;}
  }
  maxLeft = curLeft;
  maxRight = curRight;
  maxFront = curFront;
} 

//依照maxAngle轉彎
void setCourse() {
  //maxAngle=障礙物最遠的方向   往這邊走
    if (maxAngle < 90) {turnRight();} 
    if (maxAngle > 90) {turnLeft();}
    maxDist= 0;
    maxLeft = 0;
    maxRight = 0;
    maxFront = 0;
}

void checkCourse() {
  moveBackward();
  delay(500);
  moveStop();
  setCourse();
}
//依照當前伺服馬達的位置  判斷要往左/右"看"
void changePath() {
  if (pos < 90) {lookLeft();}  //右邊 有東西  轉左邊
  if (pos > 90) {lookRight();} //左邊有東西   轉右邊
}

//讀取障礙物距離
int readPing() {
  delay(70);
  unsigned int uS = sonar.ping();
  int cm = uS/US_ROUNDTRIP_CM;
  return cm;
}

//先判斷 motorset 往前走  
void checkForward() { if (motorSet=="FORWARD") {leftMotor.run(FORWARD); rightMotor.run(FORWARD); } }
//先判斷 motorset 往後走  
void checkBackward() { if (motorSet=="BACKWARD") {leftMotor.run(BACKWARD); rightMotor.run(BACKWARD); } }
//停止
void moveStop() {leftMotor.run(RELEASE); rightMotor.run(RELEASE);}

//往前走  慢慢加速
void moveForward() {
    motorSet = "FORWARD";
    leftMotor.run(FORWARD);
    rightMotor.run(FORWARD);
  
  for (speedSet = 0; speedSet < MAX_SPEED; speedSet +=2)
  {
    leftMotor.setSpeed(speedSet+MOTORS_CALIBRATION_OFFSET);
    rightMotor.setSpeed(speedSet);
    delay(5);
  }
  
}

//往後走慢慢加速
void moveBackward() {
    motorSet = "BACKWARD";
    leftMotor.run(BACKWARD);
    rightMotor.run(BACKWARD);
  for (speedSet = 0; speedSet < MAX_SPEED; speedSet +=2)
  {
    leftMotor.setSpeed(speedSet+MOTORS_CALIBRATION_OFFSET);
    rightMotor.setSpeed(speedSet);
    delay(5);
  }
}  

//轉右彎 
void turnRight() { 
  motorSet = "RIGHT";
  leftMotor.run(FORWARD);
  rightMotor.run(BACKWARD);
  delay(400);
  motorSet = "FORWARD";
  leftMotor.run(FORWARD);
  rightMotor.run(FORWARD);      
}  

//往左彎
void turnLeft() {
  motorSet = "LEFT";
  leftMotor.run(BACKWARD);
  rightMotor.run(FORWARD);
  delay(400);
  motorSet = "FORWARD";
  leftMotor.run(FORWARD);
  rightMotor.run(FORWARD);
}  

//往左右看  (轉完之後還要再檢查一次方向)
void lookRight() {rightMotor.run(BACKWARD); delay(400); rightMotor.run(FORWARD);}
void lookLeft() {leftMotor.run(BACKWARD); delay(400); leftMotor.run(FORWARD);}
