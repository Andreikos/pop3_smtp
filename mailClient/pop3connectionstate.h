#ifndef POP3CONNECTIONSTATE
#define POP3CONNECTIONSTATE
enum Pop3ConnectionState
    {
        NotConnected,
        Authorization,
        Transaction,
        Update
    };
Q_DECLARE_METATYPE(Pop3ConnectionState);
#endif // POP3CONNECTIONSTATE

