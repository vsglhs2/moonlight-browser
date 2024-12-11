#include <cstdint>
#include <iostream>
#include <ostream>
#include <stdint.h>
#include <string>

uint32_t convertUInt8ArrayToUInt32(uint8_t *bytes) {
  return (bytes[0] << 24 | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]));
}

namespace pp {
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
  VarArray(uint8_t* bytes) {
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
  uint8_t BYTES[] = {
      // 2 args
      0x0,
      0x0,
      0x0,
      0x03,
      // 1 arg - 3 bytes
      0x0,
      0x0,
      0x0,
      0x03,
      // 2 arg - 11 bytes
      0x0,
      0x0,
      0x0,
      0x0a,
      // 3 arg - 1 byte
      0x0,
      0x0,
      0x0,
      0x01,
      // "hey"
      0x68,
      0x65,
      0x79,
      // "llo, world"
      0x6C,
      0x6C,
      0x6F,
      0x2C,
      0x20,
      0x77,
      0x6F,
      0x72,
      0x6C,
      0x64,
      // true
      0x01
  };

  pp::VarArray args = pp::VarArray(BYTES);
  std::string arg0 = args.Get(0).AsString();
  std::string arg1 = args.Get(1).AsString();
  bool arg2 = args.Get(2).AsBool();

  std::cout << arg0 << " " << arg1 << " " << arg2 << std::endl;

  return 0;
} 