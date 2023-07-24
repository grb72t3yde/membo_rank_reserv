#ifndef RING_BUF_H
#define RING_BUF_H

/* Ring buf declaration */
#define RINGBUF_TYPE_DECLARE(DATATYPE, BUFTYPE) \
    typedef struct {                            \
        int size;                               \
        int front;                              \
        int rear;                               \
        DATATYPE *elements;                     \
    } BUFTYPE;

#define RINGBUF_DECLARE(BUFTYPE, BUFNAME, DATA_TYPE, SIZE) \
    extern BUFTYPE BUFNAME;                                \
    extern DATA_TYPE ringBufMemory##BUFNAME[SIZE + 1]

/* Ring buf initialization */
#define RINGBUF_INIT(BUF, SIZE, DATA_TYPE, BUFTYPE) \
        DATA_TYPE ringBufMemory##BUF[SIZE + 1];     \
        BUFTYPE BUF = {                             \
            SIZE,                                   \
            0,                                      \
            0,                                      \
            ringBufMemory##BUF                      \
        }

/* These macro can be used to check or iterate the positions of front and rear */
#define NEXT_START_INDEX(BUF) \
    (((BUF)->front != (BUF)->size) ? ((BUF)->front + 1) : 0)
#define NEXT_END_INDEX(BUF) (((BUF)->rear != (BUF)->size) ? ((BUF)->rear + 1) : 0)

/* These macro check if the buffer is empty/full */
#define is_ringbuf_empty(BUF) ((BUF)->rear == (BUF)->front)
#define is_ringbuf_full(BUF) (NEXT_END_INDEX(BUF) == (BUF)->front)

/* Enqueuing internal functions */
#define ringbuf_write_peek(BUF) ((BUF)->elements[(BUF)->rear])
#define ringbuf_write_skip(BUF)                   \
    do {                                          \
        (BUF)->rear = NEXT_END_INDEX(BUF);        \
        if (is_ringbuf_empty(BUF))                \
            (BUF)->front = NEXT_START_INDEX(BUF); \
    } while (0)

/* Dequeuing internal functions */
#define ringbuf_read_peek(BUF) &((BUF)->elements[(BUF)->front])
#define ringbuf_read_skip(BUF) ((BUF)->front = NEXT_START_INDEX(BUF))

#define ringBufWrite(BUF, ELEMENT)         \
    do {                                   \
        ringbuf_write_peek(BUF) = ELEMENT; \
        ringbuf_write_skip(BUF);           \
    } while (0)

#define ringBufRead(BUF, ELEMENT)         \
    do {                                  \
        ELEMENT = ringbuf_read_peek(BUF); \
        ringbuf_read_skip(BUF);           \
    } while (0)


#endif
