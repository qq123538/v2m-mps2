#ifndef __MACRO_H__
#define __MACRO_H__

#define CUSTOM_ASSERT( x )                                                                         \
    do                                                                                             \
    {                                                                                              \
        if ( !( x ) )                                                                              \
        {                                                                                          \
            while ( 1 )                                                                            \
            {                                                                                      \
            }                                                                                      \
        }                                                                                          \
    } while ( 0 )

#endif // __MACRO_H__
