#include "nacl.h"

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

  pp::Var var("Hello");
  pp::Var var2;

  std::cout << var.size << " " << var2.size << " " << std::endl;

  pp::VarArrayBuffer buffer;
  buffer.buffer = BYTES;
  buffer.size = sizeof(BYTES);

  pp::VarDictionary dict2;
  dict2.Set("privateKey", "This is private key");
  dict2.Set("cert", "Hello cert");

  std::cout << dict2.Bytes().size() << std::endl;

  pp::VarDictionary dict;
  dict.Set("callbackId", pp::Var(239443));
  dict.Set("type", pp::Var("success"));
  dict.Set("ret", dict2);
  dict.Set("params", buffer);

  std::cout << dict.Bytes().size() << std::endl;

  pp::VarArray args = pp::VarArray(dict.Get("params"));
  std::string arg0 = args.Get(0).AsString();
  std::string arg1 = args.Get(1).AsString();
  bool arg2 = args.Get(2).AsBool();

  std::cout << arg0 << " " << arg1 << " " << arg2 << std::endl;

  int32_t callbackId = dict.Get("callbackId").AsInt();
  std::string method = dict.Get("type").AsString();

  std::cout << callbackId << " " << method << std::endl;

  return 0;
}