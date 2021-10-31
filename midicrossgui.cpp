#include "midicrossgui.h"
#include "ui_midicrossgui.h"

//initialize
MIDICrossGUI::MIDICrossGUI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MIDICrossGUI)
{
    //set up UI
    ui->setupUi(this);
    pointerUI = this;
    qDebug() << "UI pointer:";
    qDebug() << pointerUI;

    //init connection
    InitConnect();

    //init Rtmidi device
    InitMidiDevice();

    //init timer
    InitTimer();

    //get port when launching
    GetPort();
}

//delete things when quit
MIDICrossGUI::~MIDICrossGUI()
{
    //stop retransmition if it launched
    if(transLaunched){
        StopTrans();
    }

    //delete ui
    delete ui;

    //delete Rtmidi devices
    if (device.RtmIn != nullptr){
        delete device.RtmIn;
    }
    for (unsigned int i=0;i<MAXDESTINATION;++i){
        if (device.RtmOut[i] != nullptr){
            delete device.RtmOut[i];
        }
    }

    //delete timer
    delete timerPushQueue;
    delete timerUpdateCount;
}

//initialize connection
void MIDICrossGUI::InitConnect(){

    QObject::connect(ui->pushButtonGetPort,SIGNAL(clicked()),this,SLOT(GetPort()));
    QObject::connect(ui->pushButtonClearLog,SIGNAL(clicked()),this,SLOT(ClearLog()));
    QObject::connect(ui->pushButtonAddDest,SIGNAL(clicked()),this,SLOT(AddDest()));
    QObject::connect(ui->pushButtonRemoveDest,SIGNAL(clicked()),this,SLOT(RemoveDest()));
    QObject::connect(ui->pushButtonLaunch,SIGNAL(clicked()),this,SLOT(LaunchTrans()));
    QObject::connect(ui->listWidgetInPort,SIGNAL(itemSelectionChanged()),this,SLOT(RestartTrans()));
    QObject::connect(ui->checkBoxQueueOutput,SIGNAL(stateChanged(int)),this,SLOT(SwitchQueueOutput(int)));
    QObject::connect(ui->pushButtonWebsite,SIGNAL(clicked()),this,SLOT(OpenHomePage()));

    return;
}

//initialize Rtmidi device
void MIDICrossGUI::InitMidiDevice(){

    device.RtmIn = new RtMidiIn();
    for (unsigned int i = 0;i<MAXDESTINATION;++i){
        device.RtmOut[i] = new RtMidiOut();
    }

    return;
}

//initialize timer
void MIDICrossGUI::InitTimer(){

    timerUpdateCount = new QTimer(this);
    QObject::connect(timerUpdateCount,SIGNAL(timeout()),this,SLOT(UpdateNoteCount()));
    timerUpdateCount->start(COUNTUPDATEDELAY);

    timerPushQueue = new QTimer(this);
    QObject::connect(timerPushQueue,SIGNAL(timeout()),this,SLOT(PushQueue()));
    timerPushQueue->start(QUEUEPUSHDELAY);
    return;
}

//update note count
void MIDICrossGUI::UpdateNoteCount(){
    //qDebug() << "Update Note Count:";
    //qDebug() << QString::fromStdString(std::to_string(noteCount));
    ui->lcdNumberCount->display(QString::fromStdString(std::to_string(noteCount)));
    return;
}

//send log to list widget
void MIDICrossGUI::SendLog(std::string str){

    ui->listWidgetLog->addItem(QString::fromStdString(str));
    if (ui->listWidgetLog->count() > 0){
        ui->listWidgetLog->setCurrentRow(ui->listWidgetLog->count() - 1);
    }

    return;
}

//clear log listWidget
void MIDICrossGUI::ClearLog(){
    ui->listWidgetLog->clear();
    noteCount = 0;
    return;
}

//switch output selection when checkBos state changed
void MIDICrossGUI::SwitchQueueOutput(int state){
    if (state == Qt::Checked){
        device.queueOutput = true;
    }
    else{
        device.queueOutput = false;
    }

    return;
}

//visit website
void MIDICrossGUI::OpenHomePage(){

    QDesktopServices::openUrl(QUrl(QString("https://github.com/esun-z/MIDI-Cross")));
    return;
}

//update list widget
void MIDICrossGUI::UpdateList(){

    bool selected = false;
    QString inPortName;
    if (ui->listWidgetInPort->currentRow() != -1){
        selected = true;
        inPortName = ui->listWidgetInPort->currentItem()->text();
    }
    ui->listWidgetInPort->clear();
    ui->listWidgetInPort->addItems(device.nameInPortList);
    if (selected){
        for (int i=0;i<ui->listWidgetInPort->count();++i){
            if (inPortName == ui->listWidgetInPort->item(i)->text()){
                ui->listWidgetInPort->setCurrentRow(i);
            }
        }
    }

    ui->listWidgetDest->clear();
    ui->listWidgetDest->addItems(device.selDest);

    ui->listWidgetOutPort->clear();
    bool flag;
    for (unsigned int i=0;i<device.numOutPort;++i){
        flag = true;
        for (unsigned int j=0;j<device.numDest;++j){
            if (device.nameOutPortList.at(i) == device.selDest.at(j)){
                flag = false;
                break;
            }
        }
        if (flag){
            ui->listWidgetOutPort->addItem(device.nameOutPortList.at(i));
        }
    }

    return;
}

//get midi ports
void MIDICrossGUI::GetPort(){

    qDebug() << "Getting port.";

    GetInPort();
    GetOutPort();
    UpdateList();

    qDebug() << "Finished getting port";

    return;
}

//get midi input ports
void MIDICrossGUI::GetInPort(){

    //return if Rtmidi does not work
    if (device.RtmIn == nullptr){
        SendLog("* Failed to read midi input device.");
        return;
    }

    //count MIDI input device
    try{
        device.numInPort = device.RtmIn->getPortCount();
    }
    catch(RtMidiError &error){
        error.printMessage();
    }

    if (device.numInPort == 0){

        //update device info
        device.nameInPortList.clear();

        //update ui.listWidget
        ui->listWidgetInPort->clear();

        //return if no MIDI input device available
        SendLog("* No MIDI input port found.");
        return;
    }
    else{

        //send log
        strLog = "";
        strLog += std::to_string(device.numInPort);
        strLog += " MIDI input port found.";
        SendLog(strLog);

        //get input ports name list
        std::string nameInPort;
        device.nameInPortList.clear();
        for (unsigned int i=0;i<device.numInPort;++i){

            try{
                nameInPort = device.RtmIn->getPortName(i);
            }
            catch(RtMidiError &error){
                error.printMessage();
            }

            //delete the number after port names
            nameInPort.pop_back();nameInPort.pop_back();

            //add to name list
            device.nameInPortList << QString::fromStdString(nameInPort);
        }
        /*
        //update ui.listWidget
        ui->listWidgetInPort->clear();
        ui->listWidgetInPort->addItems(device.nameInPortList);
        */
    }

    return;
}

//get midi output ports
void MIDICrossGUI::GetOutPort(){

    //return if Rtmidi does not work
    if (device.RtmOut[0] == nullptr){
        SendLog("* Failed to read midi output device.");
        return;
    }

    //count MIDI output device
    try{
        device.numOutPort = device.RtmOut[0]->getPortCount();
    }
    catch(RtMidiError &error){
        error.printMessage();
    }

    if (device.numOutPort == 0){

        //update device info
        device.nameOutPortList.clear();

        //update ui.listWidget
        ui->listWidgetOutPort->clear();

        //return if no MIDI output device available
        SendLog("* No MIDI output port found.");
        return;
    }
    else{

        //send log
        strLog = "";
        strLog += std::to_string(device.numOutPort);
        strLog += " MIDI output port found";
        SendLog(strLog);

        //get output ports name list
        std::string nameOutPort;
        device.nameOutPortList.clear();
        for (unsigned int i=0;i<device.numOutPort;++i){

            try{
                nameOutPort = device.RtmOut[0]->getPortName(i);
            }
            catch(RtMidiError &error){
                error.printMessage();
            }

            //delete the number after port names
            nameOutPort.pop_back();nameOutPort.pop_back();

            //add to name list
            device.nameOutPortList << QString::fromStdString(nameOutPort);
        }
        /*
        //update ui.listWidget
        ui->listWidgetOutPort->clear();
        ui->listWidgetOutPort->addItems(device.nameOutPortList);
        */
    }

    return;
}

//confirm that a port exist
bool MIDICrossGUI::PortExist(QString portName){

    bool flag = false;

    std::string name;
    QStringList namePortList;

    unsigned int numIn;
    try{
        numIn = device.RtmIn->getPortCount();
    }
    catch(RtMidiError &error){
        error.printMessage();
    }

    unsigned int numOut;
    try{
        numOut = device.RtmOut[0]->getPortCount();
    }
    catch(RtMidiError &error){
        error.printMessage();
    }

    if (numIn == 0 && numOut == 0){
        return false;
    }

    for (unsigned int i = 0;i<numIn;++i){
        try{
            name = device.RtmIn->getPortName(i);
        }
        catch(RtMidiError &error){
            error.printMessage();
        }

        name.pop_back();name.pop_back();
        namePortList << QString::fromStdString(name);
    }

    for (unsigned int i = 0;i<numOut;++i){
        try{
            name = device.RtmOut[0]->getPortName(i);
        }
        catch(RtMidiError &error){
            error.printMessage();
        }

        name.pop_back();name.pop_back();
        namePortList << QString::fromStdString(name);
    }

    for (int i = 0;i<namePortList.count();++i){
        if (namePortList.at(i) == portName){
            flag = true;
            break;
        }
    }

    return flag;
}

//get input port sequence with port name
int MIDICrossGUI::GetInPortSeq(QString portName){
    int seq = -1;

    if (!PortExist(portName)){
        return -1;
    }

    std::string name;
    QStringList namePortList;
    unsigned int numIn;
    try{
        numIn = device.RtmIn->getPortCount();
    }
    catch(RtMidiError &error){
        error.printMessage();
    }

    for (unsigned int i=0;i<numIn;++i){
        try{
            name = device.RtmIn->getPortName(i);
        }
        catch(RtMidiError &error){
            error.printMessage();
        }
        name.pop_back();name.pop_back();

        if (QString::fromStdString(name) == portName){
            seq = i;
            break;
        }
    }

    return seq;
}

//get output port sequence with port name
int MIDICrossGUI::GetOutPortSeq(QString portName){
    int seq = -1;

    if (!PortExist(portName)){
        return -1;
    }

    std::string name;
    QStringList namePortList;
    unsigned int numOut;
    try{
        numOut = device.RtmOut[0]->getPortCount();
    }
    catch(RtMidiError &error){
        error.printMessage();
    }

    for (unsigned int i=0;i<numOut;++i){
        try{
            name = device.RtmOut[0]->getPortName(i);
        }
        catch(RtMidiError &error){
            error.printMessage();
        }
        name.pop_back();name.pop_back();
        namePortList << QString::fromStdString(name);
    }

    for (int i=0;i<namePortList.count();++i){
        if (namePortList.at(i) == portName){
            seq = i;
        }
    }

    return seq;
}

//add destination
void MIDICrossGUI::AddDest(){

    if(ui->listWidgetOutPort->count() == 0){
        SendLog("* There is no output port to be added.");
        return;
    }
    if(ui->listWidgetOutPort->currentRow() == -1){
        SendLog("* There is no output port selected.");
        return;
    }

    bool launched = transLaunched;
    if(transLaunched){
        StopTrans();
    }

    QString portName = ui->listWidgetOutPort->currentItem()->text();

    if (PortExist(portName)){


        device.selDest << portName;
        device.numDest++;
        UpdateList();


        if(launched){
            LaunchTrans();
        }

    }
    else{
        SendLog("* Selected port does not exist.");
        return;
    }

    return;
}

//remove destination
void MIDICrossGUI::RemoveDest(){

    if (ui->listWidgetDest->count() == 0){
        SendLog("* There is no destination to be removed.");
        return;
    }
    if (ui->listWidgetDest->currentRow() == -1){
        SendLog("* There is no destination selected.");
        return;
    }

    bool launched = transLaunched;
    if(transLaunched){
        StopTrans();
    }

    device.selDest.takeAt(ui->listWidgetDest->currentRow());
    device.numDest--;
    UpdateList();

    if (launched){
        LaunchTrans();
    }

    return;
}

//Rtmidi callback function
void MIDICrossGUI::RtmCallBack(double timeStamp, std::vector<unsigned char> *message, void *userData){
    /*
    unsigned int nBytes = message->size();
    int type;
    for (unsigned int i = 0; i < nBytes; ++i) {
        type = i % 3;
        switch (type) {
            case 0:
                std::cout << "Channel = ";
                break;

            case 1:
                std::cout << "Key = ";
                break;

            case 2:
                std::cout << "Dynamic = ";
                break;

            default:
                std::cout << "Unknown value = ";
        }
        std::cout << (int)message->at(i) << "  ";
    }
    std::cout << "\nTime Stamp= " << timeStamp << "	User Data= " << userData << "\n";
    */
    if(pointerUI->device.queueOutput){
        pointerUI->AddtoQueue(*message);
    }
    else{
        for(unsigned int i=0;i<pointerUI->device.numDest;++i){
            pointerUI->device.RtmOut[i]->sendMessage(message);
        }
    }

    pointerUI->noteCount++;

    return;
}

//add a note to output queue
void MIDICrossGUI::AddtoQueue(std::vector<unsigned char> message){
    qDebug() << "Add to Queue";

    outputQueue.note[outputQueue.inPtr].message = message;
    outputQueue.inPtr++;
    outputQueue.inPtr%=MAXQUEUELENGTH;

    return;
}

//push all the notes in the queue to RtmOutput
void MIDICrossGUI::PushQueue(){


    if(outputQueue.outPtr != outputQueue.inPtr){

        for(unsigned int i=0;i<device.numDest;++i){
            device.RtmOut[i]->sendMessage(&outputQueue.note[outputQueue.outPtr].message);
        }
        /*
        unsigned int nBytes = outputQueue.note[outputQueue.outPtr].message.size();
        int type;
        for (unsigned int i=0;i<outputQueue.outPtr + 1;++i){
            if(nBytes == 0){
                qDebug() << QString::fromStdString(std::to_string(i));
                qDebug() << "outPtr O byte happens";
            }
        }
        for (unsigned int i = 0; i < nBytes; ++i) {
            type = i % 3;
            switch (type) {
                case 0:
                    std::cout << "Channel = ";
                    break;

                case 1:
                    std::cout << "Key = ";
                    break;

                case 2:
                    std::cout << "Dynamic = ";
                    break;

                default:
                    std::cout << "Unknown value = ";
            }
            std::cout << (int)outputQueue.note[outputQueue.outPtr].message.at(i) << "  ";
        }

        */
        outputQueue.outPtr++;
        outputQueue.outPtr%=MAXQUEUELENGTH;


        qDebug() << "Push Queue";
        //PushQueue();
    }

    return;
}

//launch retransmition
void MIDICrossGUI::LaunchTrans(){

    if(transLaunched){
        return;
    }

    if(!PortExist(device.selInPort)){
        SendLog("* Selected input port does not exist.");
        return;
    }

    if(device.numDest == 0){
        SendLog("* No destination.");
        return;
    }

    try{
        device.RtmIn->openPort(GetInPortSeq(device.selInPort));
    }
    catch(RtMidiError &error){
        error.printMessage();
    }
    device.RtmIn->setCallback(RtmCallBack);

    try{
        for (unsigned int i=0;i<device.numDest;++i){
            device.RtmOut[i]->openPort(GetOutPortSeq(device.selDest.at(i)));
        }
    }
    catch(RtMidiError &error){
        error.printMessage();
    }

    transLaunched = true;

    ui->pushButtonLaunch->setText("Stop");
    QObject::disconnect(ui->pushButtonLaunch,SIGNAL(clicked()),this,SLOT(LaunchTrans()));
    QObject::connect(ui->pushButtonLaunch,SIGNAL(clicked()),this,SLOT(StopTrans()));

    return;
}

//Stop retransmition
void MIDICrossGUI::StopTrans(){

    if(!transLaunched){
        return;
    }

    try{
        device.RtmIn->closePort();
        device.RtmIn->cancelCallback();

        for (unsigned int i=0;i<device.numDest;++i){
            device.RtmOut[i]->closePort();
        }
    }
    catch(RtMidiError &error){
        error.printMessage();
    }



    transLaunched = false;

    ui->pushButtonLaunch->setText("Launch");
    QObject::disconnect(ui->pushButtonLaunch,SIGNAL(clicked()),this,SLOT(StopTrans()));
    QObject::connect(ui->pushButtonLaunch,SIGNAL(clicked()),this,SLOT(LaunchTrans()));

    return;
}

//Restart retransmition when something changed
void MIDICrossGUI::RestartTrans(){

    bool launched = transLaunched;
    if(transLaunched){
        StopTrans();
    }

    if (ui->listWidgetInPort->currentRow() == -1){
        //SendLog("* There is no input port selected.");
        return;
    }

    QString namePort = ui->listWidgetInPort->currentItem()->text();

    if (!PortExist(namePort)){
        SendLog("* Selected input port does not exist.");
        return;
    }

    device.selInPort = namePort;


    if(launched){
        LaunchTrans();
    }

    return;
}
