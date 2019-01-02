
/*
  ------------------------------------------------------------------------------------------------------------------
  Project configuration
  ------------------------------------------------------------------------------------------------------------------
*/
const String project_name = "SPRINKLER"; // the name of this project

const bool enable_dhcp = true; //true = auto obtain ip address; false = will use the static ip address;
IPAddress IP_address = IPAddress(192, 168, 1, 95); // Static IP address
IPAddress dns = IPAddress(192, 168, 0, 1); // the IP Address of the modem
IPAddress subnet = IPAddress(255, 255, 255, 0); // subnet mask

const int outputs = 3; // The number of outputs you wish to control
const int output_pins[outputs] = {5, 4, 0}; // The output pin number in order
const String output_names[outputs] = {"Front", "Back", "Side"}; // The name for each button in order
const int output_durations[outputs] = { 1, 5, 10 }; // The timmer duration for each button in minutes
const bool output_invert = false; // false =  on is high, off is low; true = on in low, off is high;