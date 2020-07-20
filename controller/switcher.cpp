#include "switcher.hpp"

void Switcher::action() {
    if (m_type == Type::Start)
        start_cmd->execute(m_type);
    else if (m_type == Type::Total)
        int_cmd->execute(m_type);
    else if (m_type == Type::Internasional)
        int_cmd->execute(m_type);
}

Switcher& Switcher::start(StartCommand* cmd, Type type) {
    start_cmd = cmd;
    m_type = type;
    return *this;
}

Switcher& Switcher::total(InternasionalCommand *cmd, Type type) {
    int_cmd = cmd;
    m_type = type;
    return *this;
}

Switcher& Switcher::internasional(InternasionalCommand* cmd, Type type) {
    int_cmd = cmd;
    m_type = type;
    return *this;
}

Switcher& Switcher::nasional(NasionalCommand *cmd, Type type) {
    nas_cmd = cmd;
    m_type = type;
    return *this;
}
