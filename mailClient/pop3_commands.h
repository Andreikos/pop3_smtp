#ifndef POP3_COMMANDS
#define POP3_COMMANDS
enum POP3_Commands{
    USER_PASS = 0,
    STAT,
    RETR,
    LIST,
    DELE,
    QUIT
};
 Q_DECLARE_METATYPE(POP3_Commands);
#endif // POP3_COMMANDS

