#ifndef ARDUINO_SENSORS_H
#define ARDUINO_SENSORS_H

/**
    Utillity class for reading from arduino serial sensor output.
**/
class ArduinoSensors
{
public:
    ArduinoSensors( const char* portName );
    virtual ~ArduinoSensors();

private:

};

#endif // ARDUINO_SENSORS_H

