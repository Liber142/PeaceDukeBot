#pragma once
#include <cstring>
#include <cstddef>
struct mpg123_handle {int reads=0;};
inline constexpr int MPG123_FORCE_RATE=1,MPG123_OK=0;
inline int mpg123_init(){return 0;}inline mpg123_handle* mpg123_new(const char*,int*){return new mpg123_handle;}
inline int mpg123_param(mpg123_handle*,int,long,double){return 0;}inline size_t mpg123_outblock(mpg123_handle*){return 8;}
inline int mpg123_open(mpg123_handle*,const char*){return 0;}inline int mpg123_getformat(mpg123_handle*,long* r,int* c,int* e){*r=48000;*c=2;*e=16;return 0;}
inline int mpg123_read(mpg123_handle* h,unsigned char* p,size_t,size_t* n){if(h->reads++==0){std::memset(p,42,8);*n=3;return MPG123_OK;}*n=0;return -1;}
inline int mpg123_close(mpg123_handle*){return 0;}inline void mpg123_delete(mpg123_handle* h){delete h;}
