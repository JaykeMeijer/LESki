#include <WString.h>


class Program {
  public:
    String name;
    void init();
    void step();
};

class Rainbow: public Program {
  public:
    void init();
    void step();
};

