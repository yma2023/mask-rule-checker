#include <fstream>
#include <iostream>
#include <cstdint>
#include <iomanip>
#include <cstring>
#include <vector>
#include <map>
#include <string>
#include <cmath>

// レコードタイプの定義（君のコードに合わせた）
enum RecordType : uint8_t {
  HEADER    = 0x00,
  BGNLIB    = 0x01,
  LIBNAME   = 0x02,
  UNITS     = 0x03,
  ENDLIB    = 0x04,
  BGNSTR    = 0x05,
  STRNAME   = 0x06,
  ENDSTR    = 0x07,
  BOUNDARY  = 0x08,
  PATH      = 0x09,
  SREF      = 0x0A,
  AREF      = 0x0B,
  TEXT      = 0x0C,
  LAYER     = 0x0D,
  DATATYPE  = 0x0E,
  WIDTH     = 0x0F,
  XY        = 0x10,
  ENDEL     = 0x11,
  SNAME     = 0x12,
  COLROW    = 0x13,
  NODE      = 0x15,
  TEXTTYPE  = 0x16,
  PRESENTATION = 0x17,
  STRING    = 0x19,
  STRANS    = 0x1A,
  MAG       = 0x1B,
  ANGLE     = 0x1C,
  PATHTYPE  = 0x21,
  ELFLAGS   = 0x26,
  NODETYPE  = 0x2A,
  PROPATTR  = 0x2B,
  PROPVALUE = 0x2C,
  BOX       = 0x2D,
  BOXTYPE   = 0x2E,
  PLEX      = 0x2F,
  BGNEXTN   = 0x30,
  ENDEXTN   = 0x31
};

const std::map<uint8_t, const char*> RECORD_TYPE_MAP = {
  {0x00, "HEADER"},   {0x01, "BGNLIB"},   {0x02, "LIBNAME"},  {0x03, "UNITS"},
  {0x04, "ENDLIB"},   {0x05, "BGNSTR"},   {0x06, "STRNAME"},  {0x07, "ENDSTR"},
  {0x08, "BOUNDARY"}, {0x09, "PATH"},     {0x0A, "SREF"},     {0x0B, "AREF"},
  {0x0C, "TEXT"},     {0x0D, "LAYER"},    {0x0E, "DATATYPE"}, {0x0F, "WIDTH"},
  {0x10, "XY"},       {0x11, "ENDEL"},    {0x12, "SNAME"},    {0x13, "COLROW"},
  {0x15, "NODE"},     {0x16, "TEXTTYPE"}, {0x17, "PRESENTATION"}, {0x19, "STRING"},
  {0x1A, "STRANS"},   {0x1B, "MAG"},      {0x1C, "ANGLE"},    {0x21, "PATHTYPE"},
  {0x26, "ELFLAGS"},  {0x2A, "NODETYPE"}, {0x2B, "PROPATTR"}, {0x2C, "PROPVALUE"},
  {0x2D, "BOX"},      {0x2E, "BOXTYPE"},  {0x2F, "PLEX"},     {0x30, "BGNEXTN"},
  {0x31, "ENDEXTN"}
};

int32_t parse_int32(const uint8_t* bytes) {
  return (static_cast<int32_t>(bytes[0]) << 24) |
         (static_cast<int32_t>(bytes[1]) << 16) |
         (static_cast<int32_t>(bytes[2]) << 8)  |
         static_cast<int32_t>(bytes[3]);
}

int16_t parse_int16(const uint8_t* bytes) {
  return (static_cast<int16_t>(bytes[0]) << 8) | 
         static_cast<int16_t>(bytes[1]);
}

double parse_real64(const uint8_t* bytes) {
  uint64_t data = 0;
  for (int i = 0; i < 8; ++i) {
    data = (data << 8) | static_cast<uint64_t>(bytes[i]);
  }
  if (data == 0) return 0.0;
  
  int sign = (data >> 63) & 1;
  int exponent = ((data >> 56) & 0x7F) - 64;
  uint64_t mantissa = data & 0x00FFFFFFFFFFFFFFULL;
  
  double result = static_cast<double>(mantissa) / std::pow(2.0, 56) * std::pow(16.0, exponent);
  return sign ? -result : result;
}

std::string parse_string(const uint8_t* payload, int length) {
  std::string s(reinterpret_cast<const char*>(payload), length);
  size_t pos = s.find('\0');
  if (pos != std::string::npos) {
    s = s.substr(0, pos);
  }
  return s;
}

class GDSIIToTextConverter {
public:
  GDSIIToTextConverter(const std::string& input_path, const std::string& output_path)
    : input_path_(input_path), output_path_(output_path), indent_level_(0) {}
  
  void convert() {
    std::ifstream ifs(input_path_, std::ios::binary);
    if (!ifs) {
      throw std::runtime_error("Cannot open input file: " + input_path_);
    }
    
    std::ofstream ofs(output_path_);
    if (!ofs) {
      throw std::runtime_error("Cannot open output file: " + output_path_);
    }
    
    ofs << "# GDSII Text Dump\n";
    ofs << "# Source: " << input_path_ << "\n\n";
    
    uint8_t buffer[65536];
    
    while (ifs.read(reinterpret_cast<char*>(buffer), 4)) {
      uint16_t record_length = (static_cast<uint16_t>(buffer[0]) << 8) | 
                               static_cast<uint16_t>(buffer[1]);
      uint8_t rtype = buffer[2];
      uint8_t dtype = buffer[3];
      
      uint8_t* payload = buffer + 4;
      int payload_size = record_length - 4;
      
      if (payload_size > 0) {
        ifs.read(reinterpret_cast<char*>(payload), payload_size);
      }
      
      process_record(ofs, rtype, dtype, payload, payload_size);
    }
    
    std::cout << "Done: " << input_path_ << " -> " << output_path_ << "\n";
  }

private:
  std::string input_path_;
  std::string output_path_;
  int indent_level_;
  
  std::string indent() const {
    return std::string(indent_level_ * 2, ' ');
  }
  
  void process_record(std::ofstream& ofs, uint8_t rtype, uint8_t dtype, 
                      const uint8_t* payload, int payload_size) {
    switch (rtype) {
      case HEADER: {
        int16_t version = parse_int16(payload);
        ofs << "HEADER " << version << "\n";
        break;
      }
      
      case BGNLIB: {
        ofs << "BGNLIB\n";
        indent_level_ = 1;
        break;
      }
      
      case LIBNAME: {
        std::string name = parse_string(payload, payload_size);
        ofs << indent() << "LIBNAME \"" << name << "\"\n";
        break;
      }
      
      case UNITS: {
        double dbu_in_user = parse_real64(payload);
        double dbu_in_meter = parse_real64(payload + 8);
        ofs << indent() << "UNITS " << std::scientific << std::setprecision(6) 
            << dbu_in_user << " " << dbu_in_meter << "\n";
        ofs << std::fixed;
        break;
      }
      
      case ENDLIB: {
        indent_level_ = 0;
        ofs << "ENDLIB\n";
        break;
      }
      
      case BGNSTR: {
        ofs << "\n" << indent() << "BGNSTR\n";
        indent_level_ = 2;
        break;
      }
      
      case STRNAME: {
        std::string name = parse_string(payload, payload_size);
        ofs << indent() << "STRNAME \"" << name << "\"\n";
        break;
      }
      
      case ENDSTR: {
        indent_level_ = 1;
        ofs << indent() << "ENDSTR\n";
        break;
      }
      
      case BOUNDARY: {
        ofs << "\n" << indent() << "BOUNDARY\n";
        indent_level_ = 3;
        break;
      }
      
      case PATH: {
        ofs << "\n" << indent() << "PATH\n";
        indent_level_ = 3;
        break;
      }
      
      case SREF: {
        ofs << "\n" << indent() << "SREF\n";
        indent_level_ = 3;
        break;
      }
      
      case AREF: {
        ofs << "\n" << indent() << "AREF\n";
        indent_level_ = 3;
        break;
      }
      
      case TEXT: {
        ofs << "\n" << indent() << "TEXT\n";
        indent_level_ = 3;
        break;
      }
      
      case NODE: {
        ofs << "\n" << indent() << "NODE\n";
        indent_level_ = 3;
        break;
      }
      
      case BOX: {
        ofs << "\n" << indent() << "BOX\n";
        indent_level_ = 3;
        break;
      }
      
      case ENDEL: {
        indent_level_ = 2;
        ofs << indent() << "ENDEL\n";
        break;
      }
      
      case LAYER: {
        int16_t layer = parse_int16(payload);
        ofs << indent() << "LAYER " << layer << "\n";
        break;
      }
      
      case DATATYPE: {
        int16_t datatype = parse_int16(payload);
        ofs << indent() << "DATATYPE " << datatype << "\n";
        break;
      }
      
      case TEXTTYPE: {
        int16_t texttype = parse_int16(payload);
        ofs << indent() << "TEXTTYPE " << texttype << "\n";
        break;
      }
      
      case NODETYPE: {
        int16_t nodetype = parse_int16(payload);
        ofs << indent() << "NODETYPE " << nodetype << "\n";
        break;
      }
      
      case BOXTYPE: {
        int16_t boxtype = parse_int16(payload);
        ofs << indent() << "BOXTYPE " << boxtype << "\n";
        break;
      }
      
      case WIDTH: {
        int32_t width = parse_int32(payload);
        ofs << indent() << "WIDTH " << width << "\n";
        break;
      }
      
      case PATHTYPE: {
        int16_t pathtype = parse_int16(payload);
        ofs << indent() << "PATHTYPE " << pathtype << "\n";
        break;
      }
      
      case BGNEXTN: {
        int32_t ext = parse_int32(payload);
        ofs << indent() << "BGNEXTN " << ext << "\n";
        break;
      }
      
      case ENDEXTN: {
        int32_t ext = parse_int32(payload);
        ofs << indent() << "ENDEXTN " << ext << "\n";
        break;
      }
      
      case XY: {
        int num_points = payload_size / 8;
        ofs << indent() << "XY";
        for (int i = 0; i < num_points; ++i) {
          int32_t x = parse_int32(&payload[i * 8]);
          int32_t y = parse_int32(&payload[i * 8 + 4]);
          if (i > 0 && i % 4 == 0) {
            ofs << "\n" << indent() << "  ";
          }
          ofs << " " << x << "," << y;
        }
        ofs << "\n";
        break;
      }
      
      case SNAME: {
        std::string name = parse_string(payload, payload_size);
        ofs << indent() << "SNAME \"" << name << "\"\n";
        break;
      }
      
      case COLROW: {
        int16_t cols = parse_int16(payload);
        int16_t rows = parse_int16(payload + 2);
        ofs << indent() << "COLROW " << cols << " " << rows << "\n";
        break;
      }
      
      case STRING: {
        std::string str = parse_string(payload, payload_size);
        ofs << indent() << "STRING \"" << str << "\"\n";
        break;
      }
      
      case STRANS: {
        uint16_t flags = parse_int16(payload);
        ofs << indent() << "STRANS 0x" << std::hex << flags << std::dec << "\n";
        break;
      }
      
      case MAG: {
        double mag = parse_real64(payload);
        ofs << indent() << "MAG " << mag << "\n";
        break;
      }
      
      case ANGLE: {
        double angle = parse_real64(payload);
        ofs << indent() << "ANGLE " << angle << "\n";
        break;
      }
      
      case PRESENTATION: {
        uint16_t pres = parse_int16(payload);
        ofs << indent() << "PRESENTATION 0x" << std::hex << pres << std::dec << "\n";
        break;
      }
      
      case PROPATTR: {
        int16_t attr = parse_int16(payload);
        ofs << indent() << "PROPATTR " << attr << "\n";
        break;
      }
      
      case PROPVALUE: {
        std::string val = parse_string(payload, payload_size);
        ofs << indent() << "PROPVALUE \"" << val << "\"\n";
        break;
      }
      
      case ELFLAGS: {
        uint16_t flags = parse_int16(payload);
        ofs << indent() << "ELFLAGS 0x" << std::hex << flags << std::dec << "\n";
        break;
      }
      
      case PLEX: {
        int32_t plex = parse_int32(payload);
        ofs << indent() << "PLEX " << plex << "\n";
        break;
      }
      
      default: {
        auto it = RECORD_TYPE_MAP.find(rtype);
        if (it != RECORD_TYPE_MAP.end()) {
          ofs << indent() << it->second << " # (unhandled, size=" << payload_size << ")\n";
        } else {
          ofs << indent() << "# Unknown 0x" << std::hex << static_cast<int>(rtype) 
              << std::dec << " (size=" << payload_size << ")\n";
        }
        break;
      }
    }
  }
};


int main(int argc, char* argv[]) {
  std::string input_file = "gcd.gds";
  std::string output_file;
  
  if (argc >= 2) {
    input_file = argv[1];
  }
  if (argc >= 3) {
    output_file = argv[2];
  } else {
    size_t dot_pos = input_file.rfind('.');
    if (dot_pos != std::string::npos) {
      output_file = input_file.substr(0, dot_pos) + ".txt";
    } else {
      output_file = input_file + ".txt";
    }
  }
  
  std::cout << "=== GDSII to Text Converter ===\n";
  std::cout << "Input:  " << input_file << "\n";
  std::cout << "Output: " << output_file << "\n\n";
  
  try {
    GDSIIToTextConverter converter(input_file, output_file);
    converter.convert();
    std::cout << "Done!\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}