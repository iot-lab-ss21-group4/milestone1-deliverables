# ESP code of Group 4

* **main**: start function with initialization calls, setups and main loop
* **common**: intialization methods and shared variables
* **home_wifi**: setup of connection to access point using WiFi and handling of connection problems
* **time_management**: setup of time using SNTP, action triggering by time changes
* **publisher**: setup of MQTT publisher to remote broker, publish commands and event handler
* **subscriber**: setup of MQTT subscriber, message handling and event handler
* **mqtt_message**: collection of MQTT messages that can be sent from the broker
* **transitions**: setup of (light) barriers for event detection, ISR processing, logic of finite state machine
* **commands**: collection of commands from the advisors, optional test scripts
