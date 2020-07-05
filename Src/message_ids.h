
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
