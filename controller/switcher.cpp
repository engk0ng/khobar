#include "switcher.hpp"

void Switcher::action() {
     m_cmd->execute(m_type);
}

Switcher& Switcher::start(StartCommand* cmd, Type type) {
    m_cmd = cmd;
    m_type = type;
    return *this;
}

Switcher& Switcher::total(InternasionalCommand *cmd, Type type) {
    m_cmd = cmd;
    m_type = type;
    return *this;
}

Switcher& Switcher::internasional(InternasionalCommand* cmd, Type type) {
    m_cmd = cmd;
    m_type = type;
    return *this;
}

Switcher& Switcher::nasional(NasionalCommand *cmd, Type type) {
    m_cmd = cmd;
    m_type = type;
    return *this;
}
