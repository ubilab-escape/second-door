#ifdef SERIAL_DEBUG
#define SERIALPRINT(s, v)   { Serial.print(F(s)); Serial.print(v); }      ///< Print a string followed by a value (decimal)
#define SERIALPRINTX(s, v)  { Serial.print(F(s)); Serial.print(v, HEX); } ///< Print a string followed by a value (hex)
#define SERIALPRINTB(s, v)  { Serial.print(F(s)); Serial.print(v, BIN); } ///< Print a string followed by a value (binary)
#define SERIALPRINTS(s)     { Serial.print(F(s)); }                       ///< Print a string
#else
#define SERIALPRINT(s, v)   ///< Print a string followed by a value (decimal)
#define SERIALPRINTX(s, v)  ///< Print a string followed by a value (hex)
#define SERIALPRINTB(s, v)  ///< Print a string followed by a value (binary)
#define SERIALPRINTS(s)     ///< Print a string
#endif