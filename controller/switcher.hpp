#ifndef SWITCHER_HPP
#define SWITCHER_HPP

#include "startcommand.h"
#include "internasionalcommand.h"
#include "nasionalcommand.hpp"

class Switcher
{
public:
    Switcher() {}
    void action();
    Switcher& start(StartCommand* cmd, Type type);
    Switcher& total(InternasionalCommand* cmd, Type type);
    Switcher& internasional(InternasionalCommand* cmd, Type type);
    Switcher& nasional(NasionalCommand* cmd, Type type);

private:
    Command* m_cmd;
    Type m_type;
};

#endif // SWITCHER_HPP
