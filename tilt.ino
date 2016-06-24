#include <LiquidCrystal.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <math.h>
#define CS 10
#define MPU_addr 0x68    // I2C address of the MPU-6050  
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ, td;
int ts;
double x, y, z, Ax, Ay, Az;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
File file;
void init_LCD(void)
{
  lcd.begin(20, 4);
  lcd.setCursor(0,0);
  lcd.print(" -: Tilt Reading :- ");
}
void init_mpu6050(void)
{
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}
void init_SD_Module(void)
{
  if(!SD.begin(CS))
  {
    Serial.println("SD Card initialization failed");
    return;
  }
  if(SD.exists("log.txt"))
  {
    SD.remove("log.txt");
    Serial.println("Previous Log file removed");
  }
  file = SD.open("log.txt", FILE_WRITE);
  Serial.println("New Log File Created");
}
void setup(void)
{
  ts = 0;
  td = 0;
  Serial.begin(9600);
  while (!Serial);
  init_mpu6050();
  init_LCD();
  init_SD_Module();
  Serial.println("setup completed");
}
void read_mpu6050(void)
{
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  AcX = (Wire.read() << 8 | Wire.read());  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  AcY = (Wire.read() << 8 | Wire.read());  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = (Wire.read() << 8 | Wire.read());  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = (Wire.read() << 8 | Wire.read());  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = (Wire.read() << 8 | Wire.read());  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = (Wire.read() << 8 | Wire.read());  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = (Wire.read() << 8 | Wire.read());  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}
void write_file()
{
  String line = "";
  line += (String)ts;
  line += '.';
  line += (String)td;
  line += ":  ";
  line += (String)x;
  line += ", ";
  line += (String)y;
  line += ", ";
  line += (String)z;
  file.println(line);
}
void loop(void)
{
  read_mpu6050();
  Ax = (double)AcX / 16384.0, Ay = (double)AcY / 16384.0, Az = (double)AcZ / 16384.0;
  x = RAD_TO_DEG * atan(Ax / sqrt(Ay * Ay + Az * Az)), y = RAD_TO_DEG * atan(Ay / sqrt(Ax * Ax + Az * Az)), z = RAD_TO_DEG * atan(Az / sqrt(Ay * Ay + Ax * Ax));
  write_file();
  if(td == 0 || td == 4)
  {
    lcd.setCursor(0,1);
    lcd.print("X= ");
    lcd.print(x);
    lcd.setCursor(0,2);
    lcd.print("Y= ");
    lcd.print(y);
    lcd.setCursor(0,3);
    lcd.print("Z= ");
    lcd.print(z);
  }
  td++;
  if(td == 10)
  {
    td = 0;
    ts++;
    file.close();
    file = SD.open("log.txt", FILE_WRITE);
  }
  delay(100);
}

