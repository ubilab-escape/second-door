#include "mqtt_base.h"

class mqtt_laser : mqtt_base
{
private:
  /* data */
public:
  mqtt_laser(/* args */);
  ~mqtt_laser();
  void callback(char* topic, byte* message, unsigned int length) override;
};

mqtt_laser::mqtt_laser(/* args */)
{
}

mqtt_laser::~mqtt_laser()
{
}
