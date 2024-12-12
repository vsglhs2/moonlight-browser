#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

uint32_t convertUInt8ArrayToUInt32(uint8_t *bytes) {
  return (bytes[0] << 24 | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]));
}

void convertUInt32ToUInt8Array(uint32_t value, uint8_t *array) {
  uint8_t *bytes = (uint8_t *)&value;

  array[0] = bytes[0];
  array[1] = bytes[1];
  array[2] = bytes[2];
  array[3] = bytes[3];
}

void insertToVector(std::vector<uint8_t> &vector, uint8_t *dataPtr,
                    uint32_t size) {
  vector.insert(vector.end(), dataPtr, dataPtr + size);
}

void writeUInt32ToVector(std::vector<uint8_t> &vector, uint32_t value) {
  uint8_t bytes[4];
  convertUInt32ToUInt8Array(value, bytes);

  insertToVector(vector, bytes, sizeof(value));
}

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
  void initialize(const char *string) {
    int index = 0;

    this->size = strlen(string);
    this->bytes = new uint8_t[this->size];

    while (string[index] != 0x00) {
      this->bytes[index] = string[index];

      index++;
    }
  }

public:
  Var(char *string) { this->initialize(string); }

  Var(std::string string = "") { this->initialize(string.c_str()); }

public:
  ~Var() { delete this->bytes; }
};

class VarArrayBuffer {
public:
  size_t size;

public:
  uint8_t *buffer;

  uint8_t *Map() { return this->buffer; }
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
  VarDictionary() { this->map = std::map<std::string, std::vector<uint8_t> >(); }

public:
  std::vector<uint8_t> Bytes() {
    std::vector<uint8_t> bytes = std::vector<uint8_t>();

    writeUInt32ToVector(bytes, this->map.size());

    for (auto itr = this->map.begin(); itr != this->map.end(); itr++) {
      writeUInt32ToVector(bytes, itr->second.size());
      insertToVector(bytes, itr->second.data(), itr->second.size());
    }

    return bytes;
  }

private:
  void addEntry(const char *key, uint8_t *dataPtr, uint32_t dataSize,
                DataType dataType) {
    this->map[key] = std::vector<uint8_t>();
    std::vector<uint8_t> &vector = this->map[key];
    uint32_t keySize = strlen(key);

    writeUInt32ToVector(vector, keySize);
    writeUInt32ToVector(vector, dataSize);
    writeUInt32ToVector(vector, dataType);

    insertToVector(vector, (uint8_t *)key, keySize);
    insertToVector(vector, dataPtr, dataSize);
  }

public:
  void Set(std::string type, VarArrayBuffer buffer) {
    this->addEntry(type.c_str(), buffer.buffer, buffer.size,
                   DataType::ARRAY_BUFFER);
  }

public:
  void Set(std::string type, VarDictionary dict) {
    std::vector<uint8_t> bytes = dict.Bytes();
    this->addEntry(type.c_str(), bytes.data(), bytes.size(),
                   DataType::VAR_DICTIONARY);
  }

public:
  void Set(std::string type, Var var) {
    this->addEntry(type.c_str(), var.bytes, var.size, DataType::VAR);
  }

public:
  void Set(std::string type, uint32_t callbackId) {
    uint8_t bytes[4];
    convertUInt32ToUInt8Array(callbackId, bytes);

    this->addEntry(type.c_str(), bytes, sizeof(uint32_t), DataType::NUMBER);
  }

public:
  void Set(std::string type, char *string) {
    this->addEntry(type.c_str(), (uint8_t *)string, strlen(string),
                   DataType::STRING);
  }
};

class VarArrayItem {
private:
  uint8_t *bytes;
  uint32_t size;

public:
  VarArrayItem(uint8_t *bytes, uint32_t size) {
    this->bytes = bytes;
    this->size = size;
  }

  std::string AsString() {
    char chars[this->size + 1];

    for (uint32_t index = 0; index < this->size; index++) {
      chars[index] = bytes[index];
    }
    chars[this->size] += '\0';

    return (std::string)chars;
  }

  bool AsBool() { return (bool)this->bytes[0]; }
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
  VarArray(uint8_t *bytes) {
    const uint32_t OFFSET = sizeof(uint32_t);
    uint8_t *pointer = bytes;

    uint32_t argsAmount = convertUInt8ArrayToUInt32(bytes);
    pointer += OFFSET;

    this->sizes = new uint32_t[argsAmount];
    this->offsets = new uint32_t[argsAmount];

    for (int index = 0; index < argsAmount; index++) {
      uint32_t argSize = convertUInt8ArrayToUInt32(pointer);
      pointer += OFFSET;

      uint32_t indexOffset = pointer - bytes;
      pointer += argSize;

      this->sizes[index] = argSize;
      this->offsets[index] = indexOffset;
    }

    this->bytes = bytes;
  }

public:
  VarArrayItem Get(int index) {
    uint32_t size = this->sizes[index];
    uint32_t offset = this->offsets[index];

    std::cout << "Get: " << index << " " << size << " " << offset << std::endl;
    return VarArrayItem(this->bytes + offset, size);
  }

public:
  ~VarArray() {
    delete this->offsets;
    delete this->sizes;
  }
};
} // namespace pp

int main() {
  uint8_t BYTES[] = {// 2 args
                     0x0, 0x0, 0x0, 0x03,
                     // 1 arg - 3 bytes
                     0x0, 0x0, 0x0, 0x03,
                     // "hey"
                     0x68, 0x65, 0x79,
                     // 2 arg - 11 bytes
                     0x0, 0x0, 0x0, 0x0a,
                     // "llo, world"
                     0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64,
                     // 3 arg - 1 byte
                     0x0, 0x0, 0x0, 0x01,
                     // true
                     0x01};

  pp::VarArray args = pp::VarArray(BYTES);
  std::string arg0 = args.Get(0).AsString();
  std::string arg1 = args.Get(1).AsString();
  bool arg2 = args.Get(2).AsBool();

  std::cout << arg0 << " " << arg1 << " " << arg2 << std::endl;

  pp::Var var("Hello");
  pp::Var var2;

  std::cout << var.size << " " << var2.size << " " << std::endl;

  pp::VarArrayBuffer buffer;
  buffer.buffer = new uint8_t[10];
  buffer.size = 10;

  pp::VarDictionary dict2;
  dict2.Set("privateKey", "This is private key");
  dict2.Set("cert", "Hello cert");

  std::cout << dict2.Bytes().size() << std::endl;

  pp::VarDictionary dict;
  dict.Set("callbackId", 2349);
  dict.Set("type", pp::Var("success"));
  dict.Set("ret", dict2);

  std::cout << dict.Bytes().size() << std::endl;

  return 0;
}