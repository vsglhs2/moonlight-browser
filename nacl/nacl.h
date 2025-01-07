#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <map>
#include <netinet/in.h>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t convertUInt8ArrayToUInt32(uint8_t *bytes);
void convertUInt32ToUInt8Array(uint32_t value, uint8_t *array);
void insertToVector(std::vector<uint8_t> &vector, uint8_t *dataPtr,
                    uint32_t size);
void writeUInt32ToVector(std::vector<uint8_t> &vector, uint32_t value);

uint32_t readUInt32FromVector(std::vector<uint8_t> &vector, uint32_t offset);

namespace pp {
enum DataType {
  STRING,
  VAR,
  ARRAY_BUFFER,
  VAR_DICTIONARY,
  NUMBER,
};

class Var {
public:
  uint8_t *bytes;

public:
  size_t size;

private:
  void initializeString(const char *string);

public:
  Var(uint32_t number);

  Var(char *string);

  Var(std::string string = "");

public:
  ~Var();
};

class VarArrayBuffer {
public:
  size_t size;

public:
  uint8_t *buffer;

  uint8_t *Map();
};

class VarItem {
public:
  uint8_t *bytes;
  uint32_t size;

public:
  VarItem(uint8_t *bytes, uint32_t size);

  int32_t AsInt();

  std::string AsString();

  bool AsBool();
};

/**
  Bytes will have following structure:
  1)  4 byte number of entries
  2)  for each entry
    a)  4 byte number for key size
    b)  4 byte number for data size
    c)  4 byte number for data type
    d)  a bytes sized key
    e)  b bytes sized data
 */
class VarDictionary {
public:
  std::map<std::string, std::vector<uint8_t> > map;

public:
  VarDictionary();
public:
  std::vector<uint8_t> Bytes();

private:
  void addEntry(const char *key, uint8_t *dataPtr, uint32_t dataSize,
                DataType dataType);

public:
  void Set(std::string type, VarArrayBuffer buffer);

public:
  void Set(std::string type, VarDictionary dict);

public:
  void Set(std::string type, Var var);

public:
  void Set(std::string type, uint32_t callbackId);

public:
  void Set(std::string type, char *string);

public:
  VarItem Get(std::string type);
};

// TODO: add error handling
// TODO: figure out why lldb debugger leaks memory (maybe something wrong with code?)

/**
  bytes must have following structure:
  1)  4 byte number of arguments
  2)  for each argument
    a)  4 byte number for argument size
    b)  a bytes sized argument
 */
class VarArray {
private:
  uint32_t *sizes;
  uint32_t *offsets;
  uint8_t *bytes;

public:
  VarArray(VarItem item);

  VarArray(uint8_t *bytes);

public:
  void initialize(uint8_t *bytes);

public:
  VarItem Get(int index);

public:
  ~VarArray();
};
} // namespace pp

#ifdef __cplusplus
}
#endif