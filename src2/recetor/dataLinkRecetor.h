#ifndef DATALINKRECETOR_H_
#define DATALINKRECETOR_H_

enum FrameState {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    END
};

#endif // DATALINK_H_
