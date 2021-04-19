#include <cnc.h>

//-------------------

GAMEPAD::GAMEPAD(QWidget *parent):QObject(parent)
{
    file->open(QFile::ReadOnly | QFile::Unbuffered);
    if(!file->isOpen()) return;

    notifier = new QSocketNotifier(file->handle(),QSocketNotifier::Read,this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(handle_readNotification()));
    connect(notifier, SIGNAL(activated(int)), parent, SLOT(joypadEvent()));
}

GAMEPAD::~GAMEPAD()
{
    file->close();
}

void GAMEPAD::handle_readNotification()
{       
    file->read(data,sizeof(event));

    memcpy(&event, data, sizeof(event));

    if(event.type == EV_KEY)
    {
        if(event.code == 290)
        {            
            if     (event.value == 1) {Bpress   = true; Brelease = false;}
            else if(event.value == 0) {Brelease = true; Bpress   = false;}
        }
        else if(event.code == 289)
        {
            //if     (event.value == 1) A = true;
            //else if(event.value == 0) A = false;
        }
    }    
    else if(event.type == EV_ABS)
    {
        if(event.code == 0)
        {
            if     (event.value == 0)    left   = true;
            else if(event.value == 127) {center = true; left = false; right = false;}
            else if(event.value == 255)  right  = true;
        }
        else if(event.code == 1)
        {
            if     (event.value == 0)    up     = true;
            else if(event.value == 127) {center = true; up = false; down = false;}
            else if(event.value == 255)  down   = true;
        }
    }
}

//-------------------

SERIAL::SERIAL(QObject *parent):QSerialPort(parent)
{
    setPortName("/dev/ttyUSB0")                ;
    open(QIODevice::ReadWrite)                 ;
    setBaudRate(QSerialPort::Baud2400)         ;
    setDataBits(QSerialPort::Data8)            ;
    setParity(QSerialPort::NoParity)           ;
    setStopBits(QSerialPort::OneStop)          ;
    setFlowControl(QSerialPort::NoFlowControl) ;

    connect(this, SIGNAL(readyRead())                  , this  , SLOT(readSerial()));
    //connect(this, SIGNAL(fullDataReceived(QByteArray*)), parent, SLOT(setText(QByteArray*)));
}

SERIAL::~SERIAL()
{    
    close();
}

void SERIAL::writeSerial(QString str)
{
    char const *data = str.toStdString().c_str();
    if (isOpen() && isWritable() )
    {        
        write(data)                             ;
        flush()                                 ;
        //qDebug()<<"writeSerial       :  "<< data;
    }
}

void SERIAL::readSerial()
{
    *data = readAll()                          ;
    *dataTotal += *data                        ;
    if(data->endsWith('>'))
    {
        //emit fullDataReceived(dataTotal);
        //qDebug()<<"readSerial        : " << *dataTotal;
        strRecv = *dataTotal                   ;
        *dataTotal = ""                        ;
    }
}

//-------------------

MENU::MENU(QWidget* parent):QMenuBar(parent)
{
    addMenu(menu1);
    addMenu(menu2);
    addMenu(menu3);

    menu1->addAction(action1);
    menu1->addAction(action2);
    menu2->addAction(action3);
    menu2->addAction(action4);
    menu2->addAction(action5);
    menu2->addAction(action6);
    menu3->addAction(action7);
    menu3->addAction(action8);
    menu3->addAction(action9);

    connect(action1, SIGNAL(triggered()), parent, SLOT(eraseLastAction()));
    connect(action2, &QAction::triggered, qApp  , QApplication::quit);
    connect(action3, SIGNAL(triggered()), parent, SLOT(listToFile()));
    connect(action4, SIGNAL(triggered()), parent, SLOT(fileToList()));
    connect(action5, SIGNAL(triggered()), parent, SLOT(listToText()));
    connect(action6, SIGNAL(triggered()), parent, SLOT(textToList()));
    connect(action7, SIGNAL(triggered()), parent, SLOT(gridSizeCM()));
    connect(action8, SIGNAL(triggered()), parent, SLOT(gridSizeMM()));
    connect(action9, SIGNAL(triggered()), parent, SLOT(gridSizeM()));
}

//-------------------
TEXT::TEXT():QTextEdit()
{
    setMinimumWidth(100);
    setAlignment(Qt::AlignCenter);
}

//text->undo();
void TEXT::textToList(LIST* list, SCENE* scene)
{
    int x1, y1, x2, y2;
    int index = 0;
    QString txt = toPlainText();
    QTextStream * stream = new QTextStream(&txt, QIODevice::ReadOnly);
    QString str;
    QRegExp reg("[XYZ ]");
    QStringList stringList;
    while(!stream->atEnd())
    {
        str = stream->readLine();
        list->createItem(str);
        stringList = str.split(reg, QString::SkipEmptyParts);
        x1 = stringList.at(0).toInt();
        y1 = stringList.at(1).toInt();

        index++;

        str = stream->readLine();
        list->createItem(str);
        stringList = str.split(reg, QString::SkipEmptyParts);
        x2 = stringList.at(0).toInt();
        y2 = stringList.at(1).toInt();

        scene->createLine(x1,y1,x2,y2);
    }
}

//--------------------

LIST::LIST(QWidget *parent):QListWidget(parent)
{
    connect(itemDelegate(), SIGNAL(commitData(QWidget*)), parent, SLOT(onListCommitData(QWidget*)));
}

void LIST::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Return)
    {
        if(!pressedEvent)
        {
            editItem(currentItem());
            pressedEvent = true;
        }
    }
    else QListWidget::keyPressEvent(event);
}

void LIST::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::RightButton)
    {
        if(selectedItems().count() == 0) return;
        listItem = new QListWidgetItem("X0 Y0");
        listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
        insertItem(currentRow()+1,listItem);
        listItem = new QListWidgetItem("X0 Y0");
        listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
        insertItem(currentRow()+2,listItem);
    }
    else QListWidget::mousePressEvent(event);
}

void LIST::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton)
        if(!pressedEvent) {editItem(currentItem()); pressedEvent = true;}
}

void LIST::createItem(QString str)
{
    listItem = new QListWidgetItem(str);
    listItem->setTextAlignment(Qt::AlignLeft);
    listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
    addItem(listItem);
}

void LIST::eraseItems(int index)
{
    takeItem(index);
    takeItem(index);
}

void LIST::eraseLastItems()
{
    eraseItems(count()-1); //ERASE LAST ROW INDEX NUMBER
    eraseItems(count()-1); //ERASE LAST ROW INDEX NUMBER
}

int LIST::parseStringList(QString str, QString letter)
{
    QStringList stringList = str.split(" ", QString::SkipEmptyParts);
    foreach(QString item,stringList)
        if(item.contains(letter))return item.remove(letter).toInt();
    return 0;
}

void LIST::onListCommitData(QWidget* data, SCENE* scene)
{
    pressedEvent = false;
    int x, y;
    QString str = static_cast<QLineEdit*>(data)->text();

    x = parseStringList(str, "X");
    y = parseStringList(str, "Y");

    int listIndex = currentRow();
    if(listIndex % 2 == 1) //IF LIST ROW INDEX ODD
    {
        listIndex -= 1;
        if(str.isEmpty()){eraseItems(listIndex); scene->eraseLine(listIndex/2); return;}
        str = item(listIndex)->text();
        scene->modifyLine(parseStringList(str,"X"), parseStringList(str,"Y"), x, y, listIndex/2);
    }
    else //IF LIST ROW INDEX EVEN
    {
        if(str.isEmpty()){eraseItems(listIndex); scene->eraseLine(listIndex/2); return;}
        listIndex += 1;
        str = item(listIndex)->text();
        scene->modifyLine(x, y, parseStringList(str,"X"), parseStringList(str,"Y"), listIndex/2);
    }
}

void LIST::listToFile(QString filedir, QString filename)
{
    QDir::setCurrent(filedir);
    QFile file;
    file.setFileName(filename);

    if(file.exists()) file.open(QIODevice::WriteOnly | QIODevice::Text);
    else return;

    QString str;
    for(int listIndex=0;listIndex<count();listIndex++)
    {
        setCurrentRow(listIndex);
        str = currentItem()->text();
        file.write(str.toUtf8()); //convert To const QBytesArray
        file.write("\n");
    }
    file.close();
}

void LIST::listToText(QTextEdit* text)
{
    QString str;
    for(int listIndex=0; listIndex<count(); listIndex++)
    {
        setCurrentRow(listIndex);
        str = currentItem()->text();
        text->append(str);
    }
}

void LIST::listFromFile(QString filedir, QString filename, SCENE* scene)
{
    QFile file;
    QDir::setCurrent(filedir);
    file.setFileName(filename);

    if(file.exists()) file.open(QIODevice::ReadOnly | QIODevice::Text);
    else return;

    QString read, X, Y;
    QStringList stringList;
    QString str;
    int x1, y1, x2, y2;
    while(!file.atEnd())
    {
        read = file.readLine();
        read.remove('\n');
        stringList = read.split(' ', QString::SkipEmptyParts);
        for(int i=0;i<stringList.size();i++)
        {
            if     (stringList.at(i)[0]=='X') X = QString(stringList.at(i));
            else if(stringList.at(i)[0]=='Y') Y = QString(stringList.at(i));
        }
        createItem(X+" "+Y);
    }
    file.close();

    for(int listIndex=0;listIndex<count();listIndex++)
    {
        setCurrentRow(listIndex);
        str = currentItem()->text();
        x1 = parseStringList(str, "X");
        y1 = parseStringList(str, "Y");
        listIndex+=1;
        setCurrentRow(listIndex);
        str = currentItem()->text();
        x2 = parseStringList(str, "X");
        y2 = parseStringList(str, "Y");

        scene->createLine(x1,y1,x2,y2);
    }
}

//--------------------

SCENE::SCENE(QObject* parent):QGraphicsScene(parent)
{        
    connect(this, SIGNAL(changedMousePosition(QPointF)), parent, SLOT(onChangedMousePosition(QPointF)));
    connect(this, SIGNAL(mouseRelease(QPointF,QPointF)), parent, SLOT(onMouseRelease(QPointF,QPointF)));       
}

void SCENE::drawForeground(QPainter *painter, const QRectF &)
{
    for (int i=0;i<lines.count();i++)
    {
        painter->setPen(QPen(selectLineColor(lines.at(i)->p1(), lines.at(i)->p2()),1));
        painter->drawLine(*(lines.at(i)));        
    }
    if(p != QPointF(-1,-1))
    {
        painter->setPen(QPen(Qt::green,10));
        painter->drawPoint(p);
    }
}

void SCENE::drawBackground(QPainter *painter, const QRectF &rect)
{
    setSceneRect(QRectF(0, 0, rect.width(), rect.height()));
    QVarLengthArray<QLineF, 100> linesX;
    for (qreal x=rect.left(); x<rect.right(); x+=gridSize)
        linesX.append(QLineF(x, rect.top(), x, rect.bottom()));

    QVarLengthArray<QLineF, 100> linesY;
    for (qreal y=rect.top(); y<rect.bottom(); y+=gridSize)
        linesY.append(QLineF(rect.left(), y, rect.right(), y));

    painter->setPen(QPen(Qt::black,0.1));
    painter->drawLines(linesX.data(), linesX.size());
    painter->drawLines(linesY.data(), linesY.size());
}

void SCENE::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton)
    {
        origPoint = event->scenePos();        
        toDrawOrNot = !toDrawOrNot;       
    }
}

void SCENE::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{     
    endPoint = event->scenePos();    

    if(toDrawOrNot)
    {
        if(!startDrawLine)
        {
            startDrawLine = !startDrawLine;
            lines.append(new QLineF(origPoint.x(),origPoint.y(),origPoint.x(),origPoint.y()));
            p = pointIntersectLine(origPoint,origPoint);
        }
        lines.last()->setP2(endPoint);
        p = pointIntersectLine(origPoint,endPoint);
        update();
    }    

    emit changedMousePosition(endPoint);
}

void SCENE::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(toDrawOrNot)
    {
        startDrawLine = !startDrawLine;
        toDrawOrNot = !toDrawOrNot;
        p = pointIntersectLine(QPointF(-1,-1),QPointF(-1,-1));
        update();
        emit mouseRelease(origPoint,event->scenePos());
    }
}

void SCENE::changeGridSize(int size)
{
    gridSize = size;
    update();
}

QColor SCENE::selectLineColor(QPointF point1, QPointF point2)
{
    QColor color;

    if(int(point1.x())==int(point2.x()))
        if(int(point1.x()) % gridSize == 0)
        {
            color = Qt::magenta; //LIGNE GRILLE VERTICALE
            if(int(point2.y()) % gridSize == 0)color = Qt::cyan; //LIGNE GRILLE VERTICALE ET HORIZONTALE
        }
        else color = Qt::blue; //LIGNE VERTICALE

    else if(int(point1.y())==int(point2.y()))
        if( int(point1.y()) % gridSize == 0)
        {
            color = Qt::magenta; //LIGNE GRILLE HORIZONTALE
            if(int(point2.x()) % gridSize == 0)color = Qt::cyan; //LIGNE GRILLE HORIZONTAL ET VERTICALE
        }
        else color = Qt::red; //LIGNE HORIZONTALE

    else color = Qt::gray; //LIGNE DIAGONALE    

    return color;
}

QPointF SCENE::pointIntersectLine(QPointF point1,QPointF point2)
{        
    for(int index=0; index<lines.size(); index++)
    {
        if(lines.at(index)->intersect(QLineF(point1,point2), intersectionPoint))
        {
            int x  = int(intersectionPoint->x());
            int y  = int(intersectionPoint->y());
            int x1 = int(lines.at(index)->p1().x());
            int y1 = int(lines.at(index)->p1().y());
            int x2 = int(lines.at(index)->p2().x());
            int y2 = int(lines.at(index)->p2().y());

            if( (x == int(point2.x())) and (y == int(point2.y())) )
            {
                if( ((x1 <= x) and (x2 >= x))  or  ((x1 >= x) and (x2 <= x)) )
                    if( ((y1 <= y) and (y2 >= y))  or  ((y1 >= y) and (y2 <= y)) )
                        return point2;
            }
            else if( (x == int(point1.x())) and (y == int(point1.y())) )
            {
                if( ((x1 <= x) and (x2 >= x))  or  ((x1 >= x) and (x2 <= x)) )
                    if( ((y1 <= y) and (y2 >= y))  or  ((y1 >= y) and (y2 <= y)) )
                        return point1;
            }
        }
    }
    return QPointF(-1,-1);
}

void SCENE::eraseLine(int index)
{
    if(!lines.isEmpty())
    {
        lines.remove(index);
        update();
    }
}

void SCENE::eraseLastLine()
{    
    if(!lines.isEmpty())
    {
        lines.removeLast();        
        update();
    }
}

void SCENE::createLine(int x1,int y1,int x2,int y2)
{   
    lines.append(new QLineF(x1,y1,x2,y2));
    update();
}

void SCENE::modifyLine(int x1,int y1,int x2,int y2,int index)
{    
    lines.replace(index,new QLineF(x1,y1,x2,y2));
    update();
}

//--------------------

CNC::CNC():QWidget()  
{    
    gamepad = new GAMEPAD(this);
    text    = new TEXT();
    serial  = new SERIAL(this);
    menuBar = new MENU(this);
    list    = new LIST(this);
    scene   = new SCENE(this);
    view    = new QGraphicsView(scene);

    connect(button1, SIGNAL(clicked()), this, SLOT(eraseLastAction()));
    connect(timer  , SIGNAL(timeout()), this, SLOT(joypadEvent()));

    view->setMinimumHeight(400)                 ;
    view->setMinimumWidth(1000)                 ;
    view->setMouseTracking(true)                ;
    view->setRenderHints(QPainter::Antialiasing);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    list->setMaximumWidth(200);

    spinBoxPosX->setRange(-100, 10000);
    spinBoxPosY->setRange(-100, 10000);
    spinBoxPosZ->setRange(  10, 10000);

    layout->addWidget(menuBar     ,  0,  0,  1,  2);
    layout->addWidget(button1     ,  0,  2,  1,  8);
    layout->addWidget(spinBoxPosX ,  1,  0,  1,  2);
    layout->addWidget(spinBoxPosY ,  1,  2,  1,  2);
    layout->addWidget(spinBoxPosZ ,  1,  4,  1,  2);
    layout->addWidget(list        ,  2,  0,  1,  1);
    layout->addWidget(view        ,  2,  1,  1, 11);

    //layout->addWidget(listGcode   ,  3,  4,  1,  4);
    //layout->addWidget(text        ,  3,  8,  1,  3);
    this->setLayout(layout);
}

CNC::~CNC(){}

void CNC::joypadEvent()
{
    x = cursor->pos().x();
    y = cursor->pos().y();

    if     (gamepad->left)   {timer->start(5); cursor->setPos(--x,  y);}
    else if(gamepad->right)  {timer->start(5); cursor->setPos(++x,  y);}
    else if(gamepad->up)     {timer->start(5); cursor->setPos(  x,--y);}
    else if(gamepad->down)   {timer->start(5); cursor->setPos(  x,++y);}
    else if(gamepad->center)  timer->stop();

    globalPos = view->viewport()->mapFromGlobal(QPoint(x,y));

    if(!view->viewport()->rect().contains(globalPos)) return; //IF CURSOR NOT OVER VIEW

    else if(gamepad->Bpress)
    {
        if(startDrawLine == false)
        {
            evt = new QMouseEvent(QEvent::MouseButtonPress,globalPos,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
            qApp->sendEvent(view->viewport(),evt);
            startDrawLine = true;
        }
    }
    else if(gamepad->Brelease)
    {
        if(startDrawLine == true)
        {
            evtRelease = new QMouseEvent(QEvent::MouseButtonRelease,globalPos,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
            qApp->sendEvent(view->viewport(),evtRelease);
            startDrawLine = false;
        }
    }
}

void CNC::keyPressEvent(QKeyEvent *event)
{    
    x = cursor->pos().x();
    y = cursor->pos().y();
    globalPos = view->viewport()->mapFromGlobal(QPoint(x,y));

    if     (event->key() == Qt::Key_Q) cursor->setPos(--x,  y);
    else if(event->key() == Qt::Key_S) cursor->setPos(++x,  y);
    else if(event->key() == Qt::Key_Z) cursor->setPos(x  ,--y);
    else if(event->key() == Qt::Key_W) cursor->setPos(x  ,++y);

    if(!view->viewport()->rect().contains(globalPos)) return; //IF CURSOR NOT OVER VIEW

    else if(event->key() == Qt::Key_Return)
    {
        if(startDrawLine == false)
        {
            evt = new QMouseEvent(QEvent::MouseButtonPress,globalPos,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
            qApp->sendEvent(view->viewport(),evt);
            startDrawLine = true;
        }
        else if(startDrawLine == true)
        {
            evtRelease = new QMouseEvent(QEvent::MouseButtonRelease,globalPos,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
            qApp->sendEvent(view->viewport(),evtRelease);
            startDrawLine = false;
        }
    }
}

void CNC::onListCommitData(QWidget* data)
{
    list->onListCommitData(data, scene);
}

void CNC::onChangedMousePosition(QPointF point)
{
    spinBoxPosX->setValue(int(point.x()));
    spinBoxPosY->setValue(int(point.y()));
}

void CNC::onMouseRelease(QPointF point1, QPointF point2)
{
    QString str1("X" + QString::number(point1.x()) +" Y" + QString::number(point1.y()));
    QString str2("X" + QString::number(point2.x()) +" Y" + QString::number(point2.y()));
    list->createItem(str1);
    list->createItem(str2);
}

void CNC::gridSizeCM()
{
    scene->changeGridSize(100);
}

void CNC::gridSizeMM()
{
    scene->changeGridSize(50);
}

void CNC::gridSizeM()
{
    scene->changeGridSize(10);
}

void CNC::eraseLastAction()
{
    list->eraseLastItems();
    scene->eraseLastLine();
}

void CNC::fileToList()
{
    list->listFromFile("/home/rom1/Bureau/","fichierCNC5",scene);
}

void CNC::listToFile()
{
    list->listToFile("/home/rom1/Bureau/","fichierCNC5");
}

void CNC::listToText()
{
    list->listToText(text);
}

void CNC::textToList()
{
    text->textToList(list, scene);
}

void CNC::startTimer()
{
    connect(timer, SIGNAL(timeout()), this, SLOT(sendGcode()));
    timer->start(1000);
}

void CNC::stopTimer()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(sendGcode()));
    i_timer = 0;
}

void CNC::sendGcode()
{
    if(i_timer<list->count())
    {
        list->setCurrentRow(i_timer)                     ;
        strSend = '<' + list->currentItem()->text() + '>';
        serial->writeSerial(strSend)                     ;
        serial->waitForBytesWritten(100)                 ;
        if(QString::compare(strSend, serial->strRecv)==0)
        {
            text->append(serial->strRecv);
            i_timer++                    ;
        }
        qDebug() << "sendGcode strSend : " << strSend        ;
        qDebug() << "sendGcode strRecv : " << serial->strRecv;
    }
    else stopTimer();
}
