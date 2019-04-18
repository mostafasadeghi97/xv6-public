#include "types.h"
#include "stat.h"
#include "user.h"
 
int
main(void)
{
printf(1, "parent pid is %d\n", getppid());
exit();
}