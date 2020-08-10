#pragma once


// DEBUG
#ifdef _DEBUG
#define D_PRINTF  printf
#define GL_CHECK(x) \
    { \
    int err = glGetError(); \
    printf("GL Error = %x for %s\n", err, (char*)(#x)); \
    if (GL_NO_ERROR != err) \
    { \
        \
    }\
}
#else //DEBUG
#define D_PRINTF(x)
#define GL_CHECK(x)
#endif //DEBUG

