#ifndef Log_h
#define Log_h

/**
 * @brief Logging class
 */
class Log {
private:
  Stream* output_; /**< Pointer to stream used for output */
  bool debug_; /**< Debug mode flag */
public:
  Log();
  Log(Stream* terminal, bool debug)
  info(char message[]);
  debug(char message[]);
  error(char message[]);
  fatal(char message[]);
};

#endif
