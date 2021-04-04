/*
 * base64.h
 *
 *  Created on: Mar 3, 2021
 *      Author: Cole
 */

#ifndef BASE64_H_
#define BASE64_H_

int Base64decode_len(const char *bufcoded);
int Base64decode(char *bufplain, const char *bufcoded);

#endif /* BASE64_H_ */
