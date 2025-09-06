/*
 * as5048a.h
 *
 *  Created on: Sep 6, 2025
 *      Author: Miloush
 */

#ifdef ENABLE_SERIAL_DEBUGGING
    #define LOG(format, ...) printf(format, ##__VA_ARGS__)
#else
    #define LOG(format, ...) do {} while (0)
#endif
