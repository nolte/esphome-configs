#include "esphome.h"

const uint16_t I2C_ADDRESS = 0x21;
const uint16_t REGISTER_ADDRESS = 0x78;
const uint16_t POLLING_PERIOD = 15000; // milliseconds
char temp = 20;                        // Initial value of the register

class MyCustomComponent : public PollingComponent
{
public:
    Sensor *temperature_sensor = new Sensor();
    MyCustomComponent() : PollingComponent(POLLING_PERIOD) {}
    float get_setup_priority() const override { return esphome::setup_priority::BUS; } // Access I2C bus

    void setup() override
    {
        // Add code here as needed
        Wire.begin();
    }

    void update() override
    {
        char register_value = id(input_1).state; // Read the number set on the dashboard
        // Did the user change the input?
        if (register_value != temp)
        {
            Wire.beginTransmission(I2C_ADDRESS);
            Wire.write(REGISTER_ADDRESS);
            Wire.write(register_value);
            Wire.endTransmission();
            temp = register_value; // Swap in the new value

            Wire.beginTransmission(byte(0x55)); 	// transmit to device
            Wire.write(byte(0x41));      		// command change address 
            Wire.write(byte((0x56 << 1) & 0xFE));   	// new address, 7-bit
            Wire.endTransmission();      		// stop transmitting
            
        }

    }
};