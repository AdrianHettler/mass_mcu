
#define data_down_enviromental 100
#define data_down_power 101
#define data_down_pneumatics 102

struct enviromental
{
	float temp_inside;
	float temp_outside;
	float pressure;
};

struct power
{
	float voltage_bexus;
	float voltage_extra;
	float current_bexus;
	float current_extra;
};

struct pneumatics
{
	float pressure_tank;
	float pressure_outside_structures;
	float pressure_inside_structures;
};




#define tcp_ok 1
#define tcp_ping 2
#define tcp_pong 3
#define msg_error 254

#define exp_release_structures 21
#define exp_start_inflation 22
#define exp_uv_on 23
#define exp_uv_off 24
#define exp_valves_manual_on 27
#define exp_valves_manual_off 28
#define exp_stop_inflation 29








#define I2C_ADRESS_POWER_ADC 0x6e
