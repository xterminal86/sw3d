#include <cstdint>
#include <cstdio>

#include "blg.h"

int main(int argc, char* argv[])
{
  auto Test = [](int x1, int y1, int x2, int y2)
  {
    printf("(%d, %d) - (%d, %d)\n\n", x1, y1, x2, y2);

    BLG bg;

    bg.Init(x1, y1, x2, y2);

    BLG::Point* p = bg.Next();
    while (p != nullptr)
    {
      printf("(%d, %d)\n", p->first, p->second);
      p = bg.Next();
    }

    printf("-----\n");
  };

  Test(0, 0, 10, 10);
  Test(10, 10, 0, 0);
  Test(10, 10, 20, 9);
  Test(20, 9, 10, 10);
  Test(10, 10, 20, 11);
  Test(20, 11, 10, 10);
  Test(10, 10, 20, 15);
  Test(10, 10, 20, 5);
  Test(10, 10, 15, 20);

  return 0;
}

