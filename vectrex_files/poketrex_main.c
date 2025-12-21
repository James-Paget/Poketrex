#include "vectrex.h"
#include "vectrex/bios.h"

int main()
{
  while(1)
  {
    wait_retrace();
    intensity(0x7f);
    //pass
  }
  return 0;
}