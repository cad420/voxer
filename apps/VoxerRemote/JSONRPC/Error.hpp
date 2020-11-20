#pragma once
#include <stdexcept>

namespace voxer::remote {

enum class JSONRPCErrorCode {
  ParseError = -32700,
  InvalidRequest = -32600,
  MethodNotFound = -32601,
  InvalidParams = -32602,
  InternalError = -32603,
  ServerError = -32000,
};

class JSONRPCError : public std::exception {
public:
  JSONRPCError(int code, const std::string &message) noexcept
      : m_code(code), m_message(message),
        m_error(std::to_string(code) + ": " + message) {}

  [[nodiscard]] int code() const { return m_code; }

  [[nodiscard]] const char *what() const noexcept override {
    return m_error.c_str();
  }

protected:
  std::string m_error;

private:
  int m_code = 0;
  std::string m_message;
};

class JSONRPCParseError : public JSONRPCError {
public:
  JSONRPCParseError() : JSONRPCError(-32700, "Parse error") {}
};

class JSONRPCInvalidRequestError : public JSONRPCError {
public:
  JSONRPCInvalidRequestError() : JSONRPCError(-32600, "Invalid Request") {}
};

class JSONRPCMethodNotFoundError : public JSONRPCError {
public:
  JSONRPCMethodNotFoundError() : JSONRPCError(-32601, "Method not found") {}
};

class JSONRPCInvalidParamsError : public JSONRPCError {
public:
  JSONRPCInvalidParamsError() : JSONRPCError(-32602, "Invalid params") {}
};

class JSONRPCInternalError : public JSONRPCError {
public:
  JSONRPCInternalError() : JSONRPCError(-32603, "Internal error") {}
};

class JSONRPCServerError : public JSONRPCError {
public:
  explicit JSONRPCServerError(const std::string &msg)
      : JSONRPCError(-32000, "Server error") {
    m_error += (". " + msg);
  }
};

} // namespace voxer::remote