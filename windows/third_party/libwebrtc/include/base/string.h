#ifndef LIB_STRING_HXX
#define LIB_STRING_HXX

#ifdef LIB_WEBRTC_API_EXPORTS
#define LIB_STRING_API __declspec(dllexport)
#elif defined(LIB_WEBRTC_API_DLL)
#define LIB_STRING_API __declspec(dllimport)
#elif !defined(WIN32)
#define LIB_STRING_API __attribute__((visibility("default")))
#else
#define LIB_STRING_API
#endif

#include <string>

namespace libwebrtc {

class LIB_STRING_API string {
 public:
  string();
  string(const char* c);
  string(const char* data, unsigned size);
  string(const string& s);
  ~string();

  unsigned size() const { return length_; }

  const char* data() const { return data_; }

  string& operator=(const string& s);

  string& operator+=(const string& s);

  char operator[](unsigned j) const;
  
  char& operator[](unsigned j);

 private:
  char* data_;
  unsigned length_;
};

inline std::string to_std_string(const string& str) {
  return std::string(str.data(), str.size());
}

 inline bool operator==(const string& lhs, const string& rhs) {
  if (lhs.size() != rhs.size())
    return false;

  unsigned cap = lhs.size();
  unsigned n = 0;
  while ((n < cap) && (lhs[n] == rhs[n]))
    n++;
  return (n == cap);
}

inline bool operator==(const string& lhs, const char* rhs) {
  return (lhs == string(rhs));
}

inline bool operator==(const char* lhs, const string& rhs) {
  return (string(lhs) == rhs);
}

inline bool operator!=(const string& lhs, const string& rhs) {
  return !(lhs == rhs);
}

inline bool operator!=(const string& lhs, const char* rhs) {
  return !(lhs == rhs);
}

inline bool operator!=(const char* lhs, const string& rhs) {
  return !(lhs == rhs);
}

}  // namespace libwebrtc

#endif