#pragma once
#include <QString>

// Own only the temporary main-VFO mode selected by entering FT8/FT4.
class Ft8RadioMode {
public:
    void enter() { m_active = true; }
    void leave() { m_active = false; }
    void connectionLost() { m_originalMode = 0; m_originalSubMode = -1; }
    QString update(bool connected, int mode, int subMode, bool busy) {
        if (!connected) { connectionLost(); return {}; }
        if (busy) return {};
        if (m_active) {
            if (m_originalMode || mode == 0 || ((mode == 6 || mode == 9) && subMode < 0)) return {};
            m_originalMode = mode;
            m_originalSubMode = subMode;
            // Same DATA-A selection and readback as the existing mode/audio setup.
            return "MD6;DT0;MD;DT;LI;MG;CP;TE;";
        }
        if (!m_originalMode) return {};
        QString command = QString("MD%1;").arg(m_originalMode);
        if (m_originalSubMode >= 0) command += QString("DT%1;").arg(m_originalSubMode);
        connectionLost(); // Clear before sending: readback must not restore twice.
        return command + "MD;DT;LI;MG;CP;TE;";
    }
private:
    bool m_active = false;
    int m_originalMode = 0, m_originalSubMode = -1;
};
