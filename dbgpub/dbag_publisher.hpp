#pragma once

typedef struct {
    int32_t  block_size;
    int32_t  buffer_size;
} Options;

class dbagPublisher
{
    typedef struct CSREC {
        uint64_t nanoTime;
        long double  bidPrc;
        long double  askPrc;
        int32_t bidQty;
        int32_t askQty;
    };
    public:
        dbagPublisher();
        ~dbagPublisher();

        void init();

        void loadData(const char * url, const char * stream, Options& opts);


    private:

        CSREC csrec;

};
