#ifndef CNC_H
#define CNC_H

#include <QtWidgets>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <linux/input.h>

//--------------------

class LIST;
class SCENE;

//--------------------

class GAMEPAD : public QObject
{
    Q_OBJECT

public:
    GAMEPAD(QWidget *parent = nullptr);
    ~GAMEPAD();
    bool left = false, right = false, up = false, down = false, center = false; // AXIS FLAGS
    bool Bpress  = false, Brelease  = false; // BUTTON B  FLAGS
    bool L1press = false, L1release = false; // BUTTON L1 FLAGS
    bool R1press = false, R1release = false; // BUTTON R1 FLAGS

signals:
    void activated(int);

public slots:
    void handle_readNotification();        

protected:

private:
    QFile* file = new QFile("/dev/input/event17");
    QSocketNotifier* notifier;
    struct input_event event;
    char data[sizeof(event)];
};

//--------------------

class SERIAL : public QSerialPort
{
    Q_OBJECT

public:
    SERIAL(QObject *parent = nullptr);
    virtual ~SERIAL();    
    void writeSerial(QString);
    QString strRecv = "";

signals:
    void fullDataReceived(QByteArray*);

public slots:
    void readSerial();

private:    
    QByteArray *data = new QByteArray;
    QByteArray *dataTotal = new QByteArray;
};

//--------------------

class MENU : public QMenuBar
{
    Q_OBJECT

public:
    MENU(QWidget* parent = nullptr);

private:
    QMenu *menu1 = new QMenu("menu1");
    QMenu *menu2 = new QMenu("menu2");
    QMenu *menu3 = new QMenu("menu3");

    QAction *action1 = new QAction("Erase Last Action");
    QAction *action2 = new QAction("Quit");
    QAction *action3 = new QAction("ListToFile");
    QAction *action4 = new QAction("FileToList");
    QAction *action5 = new QAction("ListToText");
    QAction *action6 = new QAction("TextToList");
    QAction *action7 = new QAction("cm");
    QAction *action8 = new QAction("mm");
    QAction *action9 = new QAction("µm");
};

//--------------------

class TEXT : public QTextEdit
{
    Q_OBJECT

public:
    TEXT();
    void textToList(LIST*, SCENE*);

private:   

};

//--------------------

class LIST : public QListWidget
{
    Q_OBJECT

protected:
    void keyPressEvent(QKeyEvent *event);    
    void mousePressEvent(QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent *event);    

public:
    LIST(QWidget *parent = nullptr);
    void createItem(QString);
    void eraseLastItems();
    void eraseItems(int);

    void onListCommitData(QWidget*, SCENE*);
    int parseStringList(QString, QString);

    void listToFile(QString, QString);
    void listToText(QTextEdit*);
    void listFromFile(QString, QString, SCENE*);

private:
    QListWidgetItem* listItem;    

    bool pressedEvent = false;    
    void changeScene(QWidget* data, SCENE*);
};

//--------------------

class SCENE : public QGraphicsScene
{    
    Q_OBJECT

public:
    SCENE(QObject* parent = nullptr);        
    void eraseLine(int);
    void eraseLastLine();
    void createLine(int,int,int,int);
    void modifyLine(int x1,int y1,int x2,int y2,int index);
    void changeGridSize(int);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
    void drawBackground(QPainter*, const QRectF&) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

signals:
    void changedMousePosition(QPointF point);
    void mouseRelease(QPointF point1,QPointF point2);   

private:
    int gridSize = 100;
    QVarLengthArray<QLineF*,100> lines;
    QPointF origPoint;    
    QPointF endPoint;
    QPointF p = QPointF(-1,-1);
    QPointF* intersectionPoint = new QPointF;
    bool toDrawOrNot = false;
    bool startDrawLine = false;
    QColor selectLineColor(QPointF point1, QPointF point2);
    QPointF pointIntersectLine(QPointF, QPointF);
};

//--------------------

class CNC : public QWidget
{
    Q_OBJECT

public:
    CNC();
    virtual ~CNC();         
    QSpinBox* spinBoxPosX = new QSpinBox;
    QSpinBox* spinBoxPosY = new QSpinBox;
    QSpinBox* spinBoxPosZ = new QSpinBox;    
    QString   strSend     = "";

protected :
    void keyPressEvent(QKeyEvent *keyEvent);

signals:    
    void clicked();
    void timeout();

public slots:
    void onChangedMousePosition(QPointF point);
    void onMouseRelease(QPointF point1,QPointF point2);
    void eraseLastAction();
    void onListCommitData(QWidget*);
    void gridSizeCM();
    void gridSizeMM();
    void gridSizeM();

    void fileToList();
    void listToFile();

    void listToText();
    void textToList();

    void sendGcode();
    void startTimer();
    void stopTimer();

    void joypadEvent();


private:    
    QGridLayout* layout = new QGridLayout;
    QPushButton* button1 = new QPushButton("EFFACER");
    QGraphicsView* view;
    QTimer* timer = new QTimer;
    int i_timer = 0;

    QCursor *cursor = new QCursor; //MOUSE CURSOR
    QPoint globalPos;
    int x,y;
    bool startDrawLine;
    QMouseEvent *evt;              //EVENT MOUSE PRESS
    QMouseEvent *evtRelease;       //EVENT MOUSE RELEASE

    SCENE* scene;        
    LIST* list;    
    MENU* menuBar;
    GAMEPAD* gamepad;
    SERIAL* serial;
    TEXT* text;
    LIST* listGcode;        



};

#endif // CNC_H
