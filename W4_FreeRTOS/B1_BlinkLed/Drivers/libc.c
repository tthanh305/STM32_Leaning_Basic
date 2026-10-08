#include <stddef.h>

void *memset(void *ptr, int value, size_t num)
{
    unsigned char *p = (unsigned char *)ptr;

    while (num--)
    {
        *p++ = (unsigned char)value;
    }

    return ptr;
}

void *memcpy(void *dest, const void *src, size_t num)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    while (num--)
    {
        *d++ = *s++;
    }

    return dest;
}
