/*
 * File: main.h
 * Date: 02.01.2013
 * Denis Zheleznyakov aka ZiB @ http://ziblog.ru
 */

#ifndef MAIN_H_
#define MAIN_H_

#define ARRAY_LENGTH(Value)			(sizeof(Value) / sizeof(Value[0]))
#define BIT(NUMBER) (1UL << (NUMBER))

#include "stm8s.h"

#include "usb.h"

extern uint32_t watchdog_timeout;

#endif /* MAIN_H_ */
