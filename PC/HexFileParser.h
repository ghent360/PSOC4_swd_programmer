#pragma once

#include <string>
#include <list>

class HexFileParser
{
public:
  HexFileParser();
  ~HexFileParser(void);

  bool parse(std::string fileName);
  unsigned int getDataFrom(unsigned int startAddr, unsigned int maxLen, void* pBuffer);

private:
  struct Record
  {
    unsigned int adress_;
    unsigned int size_;
    unsigned char* data_;
  };

  bool process(std::istream& inputFile);
  bool processLine(const std::string& line);
  unsigned char parseHexByte(const std::string& line, int idx);
  unsigned int  parseHexWord(const std::string& line, int idx);
  void clean();
  static bool HexFileParser::compareAddress(const Record& first, const Record& second);

  unsigned int baseAddress_;
  std::list<Record> content_;
};

