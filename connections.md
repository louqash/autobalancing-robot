# Connections:

## Motor driver (TB6612FNG)
               +-------------------+
BATTERY     -> | VM           PWMA | <-     GPIO PIN 19 (RPi)
+5V         -> | VCC           AI2 | <-     GPIO PIN 21 (RPi)
0V          -> | GND           AI1 | <-     GPIO PIN 20 (RPi)
Engine      -> | A01          STBY | <-     +5V
Engine      -> | A02           BI1 | <-     GPIO PIN 26 (RPi)
Engine      -> | B01           BI2 | <-     GPIO PIN 16 (RPi)
Engine      -> | B02          PWMB | <-     GPIO PIN 18 (RPi)
0V          -> | GND           GNC | <-     0V
               +-------------------+

## IMU (MPU6050)
               +------
+5V         -> | VCC [I] (Input Voltage 3V-5V)
0V          -> | GND [I] (Ground)
GPIO PIN 3  -> | SCL [I/O] (I2C Clock pin)
GPIO PIN 2  -> | SDA [I/O] (I2C Data pin)
UNCONNECTED -> | XDA [O] (Auxiliary I2C Data pin)
UNCONNECTED -> | XCL [O] (Auxiliaxy I2C Clock pin)
UNCONNECTED -> | ADO [O] (I2C Address Select)
UNCONNECTED -> | INT [I] (Interrupt)
               +----
