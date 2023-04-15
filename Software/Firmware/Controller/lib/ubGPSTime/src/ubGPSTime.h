// ubGPSTime.h

// get utc time from u-blox gps module

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>

#define INIT_STEPS 1           // only one init step for now
#define WAIT_FOR_RESPONSE 5000 // 5 seconds

#define MAX_PAYLOAD 512
#define MAX_EXTENSIONS 4
#define EXTENSION_LEN 30

// UBX headers
const uint8_t UBX_HEADER1 = 0xB5;
const uint8_t UBX_HEADER2 = 0x62;

// UBX classes
const uint8_t UBX_NAV = 0x01;
const uint8_t UBX_ACK = 0x05;
const uint8_t UBX_CFG = 0x06;
const uint8_t UBX_MON = 0x0A;
const uint8_t UBX_NMEA = 0xF0;

// UBX message IDs
// UBX config
const uint8_t UBX_CFG_MSG = 0x01;

// UBX NMEA messages sent by default
const uint8_t UBX_NMEA_GGA = 0x00;
const uint8_t UBX_NMEA_GLL = 0x01;
const uint8_t UBX_NMEA_GSA = 0x02;
const uint8_t UBX_NMEA_GSV = 0x03;
const uint8_t UBX_NMEA_RMC = 0x04;
const uint8_t UBX_NMEA_VTG = 0x05;

// UBX MON
const uint8_t UBX_MON_VER = 0x04;

// UBX NAV
const uint8_t UBX_NAV_STATUS = 0x03;
const uint8_t UBX_NAV_TIMEUTC = 0x21;

// ACK/NACK
const uint8_t UBX_ACK_NACK = 0x00;
const uint8_t UBX_ACK_ACK = 0x01;

// UBX message
typedef struct
{
  uint8_t header1;
  uint8_t header2;
  uint8_t msgClass;
  uint8_t msgID;
  uint16_t payloadLength;
  uint8_t *payload;
  uint8_t CK_A;
  uint8_t CK_B;
} UBXMESSAGE;

// date/time information
typedef struct
{
  uint32_t timeOfWeek;
  uint32_t accuracy;
  int32_t nanoSecond;
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  bool utcValid;
  bool timeOfWeekValid;
  bool weekNumberValid;
  uint32_t timestamp;
} TIMEUTC;

// GPS status information
typedef struct
{
  uint32_t timeOfWeek;
  uint8_t gpsFixType;
  bool gpsFixOk;
  bool diffApplied;
  bool timeOfWeekValid;
  bool weekNumberValid;
  uint32_t timestamp;
} GPSSTATUS;

// GPS module information
typedef struct
{
  String swVersion;
  String hwVersion;
  String extensions[MAX_EXTENSIONS];
} MODULEVERSION;

// checksums
typedef struct
{
  uint8_t CK_A;
  uint8_t CK_B;
} CHECKSUM;

// enums
enum class direction
{
  incoming,
  outgoing
};

enum class pending
{
  none,
  version,
  ack
};

class ubGPSTime
{

protected:
  using notifyCallBack = void (*)(void *obj, UBXMESSAGE *message);

public:
  ubGPSTime();

  void attach(void *obj, notifyCallBack callBack);
  void detach();

  void initialize(bool wait);
  void begin(Stream &serialPort);

  void enableVerbose(Stream &debugPort = Serial);
  void disableVerbose();

  void process();
  void sendMessage(UBXMESSAGE *message);

  void disableDefaultNMEA();

  void setMessageRate(uint8_t msgClass, uint8_t msgID, uint8_t rate, bool wait = true);
  void pollMessage(uint8_t msgClass, uint8_t msgID);

  // single request
  void requestVersion();
  void requestStatus();
  void requestTimeUTC();

  // subscriptions
  void subscribeGPSStatus(uint8_t rate, bool wait = true);
  void subscribeTimeUTC(uint8_t rate, bool wait = true);

  MODULEVERSION getModuleVersion();
  TIMEUTC getTimeUTC();
  GPSSTATUS getGPSStatus();
  bool isInitialized();

private:
  Stream *_serialPort;
  Stream *_debugPort;
  bool _verbose;
  bool _initialized;
  pending _pending;
  bool _disabledNMEA;
  notifyCallBack _notify;
  TIMEUTC _timeUTC;
  GPSSTATUS _gpsStatus;
  MODULEVERSION _moduleVersion;
  UBXMESSAGE _message;
  uint16_t _fieldCounter;
  uint16_t _payloadCounter;
  void *_obj;

  void printMessage(UBXMESSAGE *message, direction dir);
  void printHEX(uint8_t value);

  // message processing functions
  void onAck(UBXMESSAGE *message);
  void onNack(UBXMESSAGE *message);
  void onStatus(UBXMESSAGE *message);
  void onVersion(UBXMESSAGE *message);
  void onTimeUTC(UBXMESSAGE *message);

  void processMessage(UBXMESSAGE *message);
  void onMessageEvent(UBXMESSAGE *message);
  bool waitForResponse(uint32_t timeout);

  // checksum
  void calculateChecksum(UBXMESSAGE *message, CHECKSUM *checksum);
  void stepChecksum(uint8_t value, CHECKSUM *checksum);
  bool validateChecksum(UBXMESSAGE *message);

  // field extraction functions
  uint8_t getU1(UBXMESSAGE *message, uint16_t offset);
  uint16_t getU2(UBXMESSAGE *message, uint16_t offset);
  uint32_t getU4(UBXMESSAGE *message, uint16_t offset);
  int32_t getI4(UBXMESSAGE *message, uint16_t offset);
  uint8_t getFlag(UBXMESSAGE *message, uint16_t offset, uint8_t bit);
  String getString(UBXMESSAGE *message, uint16_t offset, uint16_t length);
};
