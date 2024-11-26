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

  //
  //Test(53.4862, 10.1522, 50,      50);
  //Test(53.4862, 10.1522, 86.9233, 50);
  //
  //Test(53.8462, 10.1522, 56.7425, 13.6384);
  //Test(53.8462, 10.1522, 93.334,  13.6384);

  //Test(53.4862, 10.1522, 50, 50);
  //Test(53.4862, 10.1522, 86.92335, 50);

  //Test(53.8462, 10.1522, 56.742584, 13.6384);
  //Test(53.8462, 10.1522, 93.334, 13.6384);

  return 0;
}

