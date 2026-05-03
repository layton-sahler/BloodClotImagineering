#ifndef DATAPROCESSING_H
#define DATAPROCESSING_H

#include <QObject>
#include <QStringList>
#include <cmath>
#include <algorithm>

struct SensorMetrics {
    double stretchValue = 0.0;
    double bpm = 0.0;
    double pulse = 0.0;
    double x = 0.0, y = 0.0, z = 0.0;
    double restingScore = 100.0;
    double immobility = 0.0;
    bool isConnected = false;
    bool isMoving = false;
};

class DataProcessing : public QObject {
    Q_OBJECT
public:
    explicit DataProcessing(QObject *parent = nullptr) 
        : QObject(parent), 
          m_firstRun(true), 
          m_currentRestingScore(100.0), 
          m_currentImmobility(0.0) {}

    SensorMetrics processRawPacket(const QStringList &parts, bool connected) {
        SensorMetrics m;
        m.isConnected = connected;

        if (parts.size() >= 6) {
            m.stretchValue = parts[0].toDouble();
            m.x = parts[1].toDouble();
            m.y = parts[2].toDouble();
            m.z = parts[3].toDouble();
            m.pulse = parts[4].toDouble();
            m.bpm = parts[5].toDouble();

            if (m_firstRun) {
                m_lastX = m.x; m_lastY = m.y; m_lastZ = m.z;
                m_firstRun = false;
                m.restingScore = m_currentRestingScore;
                m.immobility = m_currentImmobility;
                return m; 
            }

            double diffX = std::abs(m.x - m_lastX);
            double diffY = std::abs(m.y - m_lastY);
            double diffZ = std::abs(m.z - m_lastZ);
            double totalChange = diffX + diffY + diffZ;
            m.isMoving = (totalChange > m_threshold);

            if (m.isMoving) {
                m_currentImmobility = std::max(0.0, m_currentImmobility - 0.05); 
            } else {
                m_currentImmobility = std::min(100.0, m_currentImmobility + 0.005);
            }

            if (m.isMoving) {
                m_currentRestingScore = std::max(0.0, m_currentRestingScore - 3.0);
            } else {
                m_currentRestingScore = std::min(100.0, m_currentRestingScore + 0.5);
            }

            m_lastX = m.x; m_lastY = m.y; m_lastZ = m.z;
        }

        m.restingScore = m_currentRestingScore;
        m.immobility = m_currentImmobility; 
        return m;
    }

private:
    bool m_firstRun;
    double m_lastX = 0.0, m_lastY = 0.0, m_lastZ = 0.0;
    double m_currentRestingScore;
    double m_currentImmobility;
    const double m_threshold = 0.95;
};

#endif