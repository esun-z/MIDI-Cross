#ifndef MIDICROSSGUI_H
#define MIDICROSSGUI_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <RtMidi.h>

#define MAXDESTINATION 512

#define COUNTUPDATETIME 1000

QT_BEGIN_NAMESPACE
namespace Ui { class MIDICrossGUI; }
QT_END_NAMESPACE

class MIDICrossGUI : public QWidget
{
    Q_OBJECT

public:
    MIDICrossGUI(QWidget *parent = nullptr);
    ~MIDICrossGUI();

    Ui::MIDICrossGUI *ui;

    struct MIDIDEVICE{
        RtMidiIn *RtmIn;
        RtMidiOut *RtmOut[MAXDESTINATION];

        unsigned int numInPort;
        unsigned int numOutPort;
        unsigned int numDest = 0;

        QString selInPort;
        QStringList selDest;

        QStringList nameInPortList;
        QStringList nameOutPortList;
    };
    MIDIDEVICE device;

    std::string strLog;
    bool transLaunched = false;

    static void RtmCallBack(double timeStamp, std::vector< unsigned char > *message, void *userData);

    unsigned long long noteCount = 0;

public slots:
    void GetPort();
    void ClearLog();
    void AddDest();
    void RemoveDest();
    void LaunchTrans();
    void StopTrans();
    void RestartTrans();
    void UpdateNoteCount();

private:
    void InitConnect();
    void InitMidiDevice();
    void InitTimer();

    void SendLog(std::string str);
    void GetInPort();
    void GetOutPort();

    bool PortExist(QString portName);
    int GetInPortSeq(QString portName);
    int GetOutPortSeq(QString portName);

    void UpdateList();

    QTimer *timerUpdateCount;
};


static MIDICrossGUI *pointerUI;

#endif // MIDICROSSGUI_H
