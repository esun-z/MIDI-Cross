#ifndef MIDICROSSGUI_H
#define MIDICROSSGUI_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>

#include <RtMidi.h>

#define MAXDESTINATION 128
#define COUNTUPDATEDELAY 1000
#define QUEUEPUSHDELAY 1
#define MAXQUEUELENGTH 512

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

        bool queueOutput = true;
    };
    MIDIDEVICE device;

    struct NOTE{
        std::vector<unsigned char> message;
    };

    struct NOTEQUEUE{
        NOTE note[MAXQUEUELENGTH];
        unsigned int inPtr = 0;
        unsigned int outPtr = 0;
    };
    NOTEQUEUE outputQueue;

    std::string strLog;
    bool transLaunched = false;
    unsigned long long noteCount = 0;

    static void RtmCallBack(double timeStamp, std::vector<unsigned char> *message, void *userData);

    void AddtoQueue(std::vector<unsigned char> message);


public slots:
    void GetPort();
    void ClearLog();
    void AddDest();
    void RemoveDest();
    void LaunchTrans();
    void StopTrans();
    void RestartTrans();
    void UpdateNoteCount();
    void SwitchQueueOutput(int state);
    void PushQueue();
    void OpenHomePage();

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
    QTimer *timerPushQueue;
};


static MIDICrossGUI *pointerUI;

#endif // MIDICROSSGUI_H
