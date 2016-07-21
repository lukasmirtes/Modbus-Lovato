#ifndef GLOBALS_H
#define GLOBALS_H

#include <QSharedPointer>

// === Macro stringification helpers ===
#define xstr(s) str(s)
#define str(s) #s

// === Timings ===
#define MODBUS_MAXRETRIES_DEFAULT	5
#define MODBUS_INITIALREADTIMEOUT_DEFAULT	100
#define MODBUS_CONSEQUENTREADTIMEOUT_DEFAULT	5

// === Settings keys ===
// --- Modbus related ---
#define MODBUS_GROUP_NAME "Modbus"
#define MODBUS_MAXRETRIES_KEY "MaxRetries"
#define MODBUS_INITIALREADTIMEOUT_KEY "InitialReadTimeout"
#define MODBUS_CONSEQUENTREADTIMEOUT_KEY "ConsequentReadTimeout"

// --- RequestGroups related ---
#define REQUEST_GROUPS_KEY "RequestGroups"
#define REQUEST_ACTIVITY_KEY "active"
#define REQUEST_DEVICE_KEY "device"
#define REQUEST_ADDRESS_KEY "address"
#define REQUEST_DATA_TYPE_KEY "dataType"
#define REQUEST_DATA_TYPE_VALUE_FLOAT float
#define REQUEST_DATA_TYPE_VALUE_DOUBLE double
#define REQUEST_BYTES_PER_ITEM "bytesPerItem"
#define REQUEST_REGISTER_COUNT "registerCount"
#define REQUEST_PERIOD_KEY "period"

#define REQUEST_ARRAY_ITEM_KEY "item"
#define REQUEST_ITEM_NAME_KEY "name"
#define REQUEST_ITEM_PDU_INDEX_KEY "pduIndex"
#define REQUEST_ITEM_PDU_OFFSET_KEY "pduOffset"
#define REQUEST_ITEM_MULTIPLIER_KEY "multiplier"
#define REQUEST_ITEM_DIVIDER_KEY "divider"
#define REQUEST_ITEM_SIGNUM_INDEX_KEY "signumIndex"

#define REQUEST_ARRAY_PARSING_KEY "parsing"
#define REQUEST_PARSING_TYPE_KEY "type"
#define REQUEST_PARSING_TYPE_VALUE_INVALID invalid
#define REQUEST_PARSING_TYPE_VALUE_POST post
#define REQUEST_PARSING_TYPE_CONSTANT_POST 1
#define REQUEST_PARSING_TYPE_VALUE_LOG log
#define REQUEST_PARSING_TYPE_CONSTANT_LOG 2

// === Object names/prefixes ===
#define REQUEST_MANAGER_NAME_PREFIX "RequestManagerOf"

// === Global datatypes ===
typedef QSharedPointer<class ProtocolDataUnit> PDUSharedPtr_t;
typedef QSharedPointer<class ApplicationDataUnitSerial> ADUSharedPtr_t;

// === Global variables ===
extern QSharedPointer<class ModbusSerialMaster> serialMaster;

#endif // GLOBALS_H
