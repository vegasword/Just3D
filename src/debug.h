#if DEBUG
void LogEx(const char *format, char *filePath, int fileLine, char *function, ...);
#define Log(format, ...) LogEx(format, __FILE__, __LINE__, __func__, __VA_ARGS__)
void PrintEx(const char *format, ...);
#define Print(format, ...) PrintEx(format, __VA_ARGS__)
#else
#define Log(format, ...) ;
#define Print(format, ...) ;
#endif

