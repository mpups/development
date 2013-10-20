#ifndef PUPPYBOTDATA_H
#define PUPPYBOTDATA_H

struct AvInfo
{
    timespec sendStamp;
    int32_t frameNumber;
};

template<typename T>
void serialize(T& archive, AvInfo& info)
{
    archive(info.sendStamp,info.frameNumber);
}

#endif // PUPPYBOTDATA_H
