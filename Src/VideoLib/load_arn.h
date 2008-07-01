#ifndef __LOAD_ARN_H_
#define __LOAD_ARN_H_

#include <stdio.h>
#include <stdlib.h>
#include <vector>

class ArnObject;
class ArnNode;

int load_arn(const char* filename, std::vector<ArnObject*>& objects);

#endif // #ifndef __LOAD_ARN_H_