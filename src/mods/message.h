#ifndef MESSAGE_H
#define MESSAGE_H 1

typedef struct Message *Message;

Message message_new(const char *);
size_t  message_serialize(Message, unsigned char **);
Message message_deserialize(unsigned char *, int);

#endif
