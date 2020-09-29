// INA219_example.cpp

#include <Arduino.h>
#include <EEPROM.h> // Should not be necessary

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "signalk/signalk_output.h"
#include "sensors/ina219.h"
#include "sensors/analog_input.h"
#include "transforms/analogvoltage.h"
#include "transforms/linear.h"

ReactESP app([] () {
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small delay and one debugI() are required so that
  // the serial output displays everything
  delay(100);
  Debug.setSerialEnabled(true);
  #endif
  delay(100);
  debugI("Serial debug enabled");

   // Create a builder object
  SensESPAppBuilder builder;

  // Create the global SensESPApp() object.
  sensesp_app = builder.set_hostname("bank2-monitor")
                    ->set_sk_server("192.168.0.1", 3000)
                    ->set_standard_sensors(IP_ADDRESS)
                    ->get_app();

  // Create an INA219, which represents the physical sensor.
  // 0x40 is the default address. Chips can be modified to use 0x41 (shown here), 0x44, or 0x45.
  // The default volt and amp ranges are 32V and 2A (cal32_2). Here, 32v and 1A is specified with cal32_1.
  auto* pINA219 = new INA219(0x40, cal16_400);


  // Define the read_delay you're going to use, if other than the default of 500 ms.
  const uint read_delay = 1000; // once per second

  // Create an INA219value, which is used to read a specific value from the INA219, and send its output
  // to SignalK as a number (float). This one is for the bus voltage.
  auto* pINA219busVoltage = new INA219value(pINA219, bus_voltage, read_delay, "/bank2/busVoltage");
      
      pINA219busVoltage->connectTo(new SKOutputNumber("electrical.bank2.busVoltage"));

  // Do the same for the shunt voltage.
  const float shuntVoltageMultiplier = 1.0; 
  const float shuntVoltageOffset = 0.;
  
  auto* pINA219shuntVoltage = new INA219value(pINA219, shunt_voltage, read_delay, "/bank2/shuntVoltage");
     
      pINA219shuntVoltage->connectTo(new Linear(shuntVoltageMultiplier, shuntVoltageOffset, "/bank2/shuntVoltageLinear"))
                         ->connectTo(new SKOutputNumber("electrical.bank2.shuntVoltage"));

  // Do the same for the current (amperage).
  const float currentMultiplier = 1.0; 
  const float currentOffset = 0.;
  auto* pINA219current = new INA219value(pINA219, current, read_delay, "/bank2/current");
      
      pINA219current->connectTo(new Linear(currentMultiplier, currentOffset, "/bank2/currentLinear"))
                    ->connectTo(new SKOutputNumber("electrical.bank2.current"));   

  // Do the same for the power (watts).
  auto* pINA219power = new INA219value(pINA219, power, read_delay, "/bank2/power");
      
      pINA219power->connectTo(new SKOutputNumber("electrical.bank2.power"));  

  // Do the same for the load voltage.
  auto* pINA219loadVoltage = new INA219value(pINA219, load_voltage, read_delay, "/bank2/loadVoltage");
      
      pINA219loadVoltage->connectTo(new SKOutputNumber("electrical.bank2.loadVoltage"));    

  uint8_t pin = A0;

  const float batteryVoltageMultiplier = 15.0; // 100% divided by 730 = 0.137 "percent per analogRead() unit"
  const float batteryVoltageOffset = 0.;
  
  auto* pAnalogInput = new AnalogInput(pin, read_delay, "/bank2/batteryVoltage");  
    pAnalogInput->connectTo(new AnalogVoltage(1.0, 1.0, 0))
                ->connectTo(new Linear(batteryVoltageMultiplier, batteryVoltageOffset, "/bank2/batteryVoltageLinear"))
                ->connectTo(new SKOutputNumber("electrical.bank2.batteryVoltage"));


  sensesp_app->enable();
});
