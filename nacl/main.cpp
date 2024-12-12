#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

uint32_t convertUInt8ArrayToUInt32(uint8_t *bytes) {
  return (bytes[0] << 24 | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]));
}

uint8_t *convertUInt32ToUInt8Array(uint32_t value, uint8_t *array) {
  uint8_t *bytes = (uint8_t *)&value;

  array[0] = bytes[0];
  array[1] = bytes[1];
  array[2] = bytes[2];
  array[3] = bytes[3];
}

namespace pp {
class Var {
public:
  uint8_t *bytes;

public:
  size_t size;

public:
  Var(char *string = "") {
    int index = 0;

    this->size = strlen(string);
    this->bytes = new uint8_t[this->size];

    while (string[index] != 0x00) {
      this->bytes[index] = string[index];

      index++;
    }
  }

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

class VarDictionary {
public:
  std::vector<uint8_t> ret;

public:
  std::vector<uint8_t> callbackId;

public:
  std::vector<uint8_t> type;

public:
  VarDictionary() {
    this->type = std::vector<uint8_t>();
    this->callbackId = std::vector<uint8_t>();
    this->ret = std::vector<uint8_t>();
  }

public:
  std::vector<uint8_t> Bytes() {
    std::cout << this->callbackId.size() << " " << this->type.size() << " "
              << this->ret.size() << std::endl;

    std::vector<uint8_t> bytes = std::vector<uint8_t>();
    bytes.insert(bytes.end(), this->callbackId.begin(), this->callbackId.end());
    bytes.insert(bytes.end(), this->type.begin(), this->type.end());
    bytes.insert(bytes.end(), this->ret.begin(), this->ret.end());

    return bytes;
  }

public:
  void Set(std::string type, VarArrayBuffer buffer) {
    if (type == "ret") {
      uint8_t size[4];
      convertUInt32ToUInt8Array(buffer.size, size);

      this->ret.insert(this->ret.end(), size, size + sizeof(uint32_t));
      this->ret.insert(this->ret.end(), buffer.buffer,
                       buffer.buffer + buffer.size);
    }
  }

public:
  void Set(std::string type, VarDictionary dict) {
    if (type == "ret") {
      std::vector<uint8_t> bytes = dict.Bytes();

      uint8_t size[4];
      convertUInt32ToUInt8Array(bytes.size(), size);

      this->ret.insert(this->ret.end(), size, size + sizeof(uint32_t));
      this->ret.insert(this->ret.end(), bytes.begin(),
                       bytes.begin() + bytes.size());
    }
  }

public:
  void Set(std::string type, Var var) {
    if (type == "type") {
      uint8_t size[4];
      convertUInt32ToUInt8Array(var.size, size);

      this->type.insert(this->type.end(), size, size + sizeof(uint32_t));
      this->type.insert(this->type.end(), var.bytes, var.bytes + var.size);
    }

    if (type == "ret") {
      uint8_t size[4];
      convertUInt32ToUInt8Array(var.size, size);

      this->ret.insert(this->ret.end(), size, size + sizeof(uint32_t));
      this->ret.insert(this->ret.end(), var.bytes, var.bytes + var.size);
    }
  }

public:
  void Set(std::string type, uint32_t callbackId) {

    if (type == "callbackId") {
      uint8_t size[4];
      convertUInt32ToUInt8Array(4, size);

      uint8_t bytes[4];
      convertUInt32ToUInt8Array(callbackId, bytes);

      this->callbackId.insert(this->callbackId.end(), size,
                              size + sizeof(uint32_t));
      this->callbackId.insert(this->callbackId.end(), bytes,
                              bytes + sizeof(uint32_t));
    }
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

      this->sizes[index] = argSize;
    }

    for (int index = 0; index < argsAmount; index++) {
      uint32_t indexOffset = pointer - bytes;
      this->offsets[index] = indexOffset;

      pointer += this->sizes[index];
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
                     // 2 arg - 11 bytes
                     0x0, 0x0, 0x0, 0x0a,
                     // 3 arg - 1 byte
                     0x0, 0x0, 0x0, 0x01,
                     // "hey"
                     0x68, 0x65, 0x79,
                     // "llo, world"
                     0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64,
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

  pp::VarDictionary dict;
  dict.Set("callbackId", 2349);
  dict.Set("type", pp::Var("success"));
  dict.Set("ret", buffer);

  std::cout << dict.Bytes().size() << std::endl;

  pp::VarDictionary dict2;
  dict2.Set("callbackId", 28849);
  dict2.Set("type", pp::Var("reject"));
  dict2.Set("ret", dict);

  std::cout << dict2.Bytes().size() << std::endl;

  return 0;
}