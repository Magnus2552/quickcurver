#include "game.h"

Game::Game(QQuickItem *parent) : QQuickItem(parent) {
	segment::initRand();
	for (int i = 0; i < MAXPLAYERCOUNT; i++) {
		controls[i][0] = Qt::Key_Left;
		controls[i][1] = Qt::Key_Right;
		controlledByAI[i] = false;
		controlledByNetwork[i] = false;
	}
    server = new Server(curver, 52552, this);
    connect(server, SIGNAL(playerStatusChanged(int,QString)), this, SLOT(setPlayerStatus(int,QString)));
}

Game::~Game() {
    if (timer != NULL) {
        timer->stop();
    }
    if (node != NULL) {
        node->removeAllChildNodes();
        delete node;
    }
}

QSGNode *Game::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) {
	(void) oldNode; //suppresses unused variable warning
	return node;
}

void Game::start() {
    timer = new QTimer(this);
    nextRoundTimer = new QTimer(this);
    node = new QSGNode;
    setFlag(ItemHasContents);
    wall = new wallNode(node, fieldsize);
    for (int i = 0; i < ITEMVARIETY; i++) {
        itemPrioritySum += itemPriority[i];
    }
    for (int i = 0; i < playercount; i++) {
        alive[i] = true;
        score[i] = 0;
        roundScore[i] = 0;
        curver[i] = new QCurver(node, colors[i], baseSpeed, fieldsize);
        if (controlledByAI[i]) {
            ai[i] = new AIController(curver[i], curver, playercount, fieldsize);
        }
        connect(curver[i], SIGNAL(died(QCurver*)), this, SLOT(curverDied(QCurver*)));
        connect(curver[i], SIGNAL(requestIntersectionChecking(QPointF,QPointF)), this, SLOT(checkforIntersection(QPointF,QPointF)));
    }
    for (int i = 0; i < MAXITEMCOUNT; i++) {
        items[i] = NULL;
    }
    connect(timer, SIGNAL(timeout()), this, SLOT(progress()));
    lastTime = QTime::currentTime();
    lastItemSpawn = lastTime;
    nextItemSpawn = segment::randInt(5000,10000);
    server->setPlayerCount(playercount);
    server->start();
    timer->start(timerInterval);
}

void Game::clientStart(QString ip, int port) {
    server->shutdown();
    host = false;
    node = new QSGNode;
    wall = new wallNode(node, fieldsize);
    if (client != NULL) {
        delete client;
    }
    client = new Client(ip, node, port, this);
    connect(client, SIGNAL(joinStatusChanged(QString)), this, SLOT(setJoinStatus(QString)));
    connect(client, SIGNAL(updateGUI()), this, SLOT(updateGUI()));
    setFlag(ItemHasContents);
}

void Game::setColor(int index, QColor color) {
	colors[index] = color;
}

void Game::progress() {
	//calculate time since last progress()
    float deltat = (float) lastTime.msecsTo(QTime::currentTime())/1000 * effectiveTimeMultiplier;
    lastTime = QTime::currentTime();
    if (lastItemSpawn.msecsTo(lastTime) > nextItemSpawn) {
        qDebug() << "Item spawned";
        int i;
        for (i = 0; items[i] != NULL; i++) { //find first free item slot
        }
        int r = segment::randInt(1, itemPrioritySum);
        int itemSelector = 0;
        for (itemSelector; r > 0; r -= itemPriority[itemSelector], itemSelector++) {};
        itemSelector--;
        switch (itemSelector) {
        case 0:
            items[i] = new FastItem(node, fieldsize);
            break;
        case 1:
//			items[i] = new SlowItem(node);
            break;
        case 2:
            items[i] = new CleaninstallItem(node, fieldsize);
            break;
        case 3:
            //global wall hack
            break;
        case 4:
            //solo wall hack
            break;
        case 5:
            items[i] = new FatterItem(node, fieldsize);
            break;
        default:
            break;
        }
        items[i]->setRound(roundCount);
        lastItemSpawn = lastTime;
        nextItemSpawn = segment::randInt(1000,10000);
    }
    for (int i = 0; i < playercount; i++) {
        if (alive[i]) {
            //check for item collision
            for (int j = 0; j < MAXITEMCOUNT; j++) {
                if (items[j] != NULL && items[j]->testCollision(curver[i]->getPos())) {
                    //use item
                    items[j]->useItem(playercount, curver, curver[i]);
                    items[j] = NULL; //dont worry it will delete it by its own
                }
            }
            //let the AI make its move now
            if (controlledByAI[i] && ((frameCount+i) % AIINTERVAL == 0)) {
                ai[i]->makeMove(deltat);
            }
            curver[i]->progress(deltat);
        }
    }
    frameCount++;
    update();
}

void Game::sendKey(Qt::Key k) {
	if (host) {
		for (int i = 0; i < playercount; i++) {
			if (!controlledByAI[i] && !controlledByNetwork[i]) {
				if (controls[i][0] == k) {
					curver[i]->rotating = ROTATE_LEFT;
				} else if (controls[i][1] == k) {
					curver[i]->rotating = ROTATE_RIGHT;
				}
			}
		}
	} else { //client
		client->sendKey(k);
	}
}

void Game::releaseKey(Qt::Key k) {
	if (host) {
		for (int i = 0; i < playercount; i++) {
			if (!controlledByAI[i] && !controlledByNetwork[i]) {
				if (controls[i][0] == k || controls[i][1] == k) {
					curver[i]->rotating = ROTATE_NONE;
				}
			}
		}
	} else { //client
		client->releaseKey(k);
	}
}

void Game::addPlayer() {
	playercount++;
}

void Game::curverDied(QCurver *who) {
	int i;
    bool onlyBotsAlive = true;
	for (i = 0; curver[i] != who; i++) {
		if (alive[i]) //if he is still alive, increase his score
			increaseScore(i);
	}
	alive[i] = false;
	qDebug() << names[i] + " died";
	i++;
	for (; i < playercount; i++) {
		if (alive[i])
			increaseScore(i);
	}
	int stillAlive = 0;
	int alivePlayer;
	for (i = 0; i < playercount; i++) {
		if (alive[i]) {
			stillAlive++;
			alivePlayer = i;
            if (!controlledByAI[i]) {
                onlyBotsAlive = false;
            }
		}
	}
    if (onlyBotsAlive) {
        effectiveTimeMultiplier = timeMultiplier;
    }
	if (stillAlive == 1) {
		qDebug() << names[alivePlayer] + " has won!";
		nextRound();
	}
}

void Game::checkforIntersection(QPointF a, QPointF b) {
	QCurver* who = (QCurver*) QObject::sender();
	bool c = false;
	for (int i = 0; i < playercount && !c; i++) {
		c = curver[i]->checkforIntersection(a, b);
	}
	if (c) {
		emit who->died(who);
	}
}



void Game::setControls(int index, Qt::Key k, bool isRight) {
	controls[index][isRight] = k;
	qDebug() << "Assigned key to player "<< index <<": " + QKeySequence(k).toString();
}

void Game::nextRound() {
    effectiveTimeMultiplier = 1;
	nextRoundTimer->singleShot(roundTimeout, this, SLOT(startNextRound()));
}

void Game::startNextRound() {
    effectiveTimeMultiplier = 1;
	timer->stop();
    server->newRound();
	roundCount++;
	for (int i = 0; i < playercount; i++) {
		roundScore[i] = 0;
		QVariant returnedValue;
		QMetaObject::invokeMethod(qmlobject, "changeScore", Q_RETURN_ARG(QVariant, returnedValue), Q_ARG(QVariant, i) , Q_ARG(QVariant, score[i]), Q_ARG(QVariant, roundScore[i]));
		curver[i]->reset();
		alive[i] = true;
	}
	for (int i = 0; i < MAXITEMCOUNT; i++) {
		if (items[i] != NULL) {
			delete items[i];
			items[i] = NULL;
		}
	}
	lastItemSpawn = QTime::currentTime();
	timer->start();
}

void Game::setName(int index, QString newName) {
	names[index] = newName;
}

void Game::setRoundTimeout(int seconds) {
	roundTimeout = 1000*seconds;
}

void Game::setBaseSpeed(int baseSpeed) {
	this->baseSpeed = baseSpeed;
}

void Game::setQmlObject(QObject *o) {
	qmlobject = o;
}

void Game::increaseScore(int index) {
	score[index]++;
	roundScore[index]++;
	QVariant returnedValue;
	QMetaObject::invokeMethod(qmlobject, "changeScore", Q_RETURN_ARG(QVariant, returnedValue), Q_ARG(QVariant, index) , Q_ARG(QVariant, score[index]), Q_ARG(QVariant, roundScore[index]));
}

void Game::setController(int index, int newControllerState) {
	controlledByAI[index] = (newControllerState == 1);
	controlledByNetwork[index] = (newControllerState == 2);
	server->setAvailable(index, controlledByNetwork[index]);
}

void Game::setItemPriority(int index, int newPriority) {
	itemPriority[index] = newPriority;
}

void Game::setJoinStatus(QString s) {
	QVariant returnedValue;
	QMetaObject::invokeMethod(qmlobject, "setJoinStatus", Q_RETURN_ARG(QVariant, returnedValue), Q_ARG(QVariant, s));
}

void Game::close() {
	delete this;
}

void Game::updateGUI() {
    update();
}

void Game::setFieldSize(int s) {
    fieldsize = s;
}

void Game::setPlayerStatus(int index, QString s) {
    QVariant returnedValue;
    QMetaObject::invokeMethod(qmlobject, "setPlayerStatus", Q_RETURN_ARG(QVariant, returnedValue), Q_ARG(QVariant, index), Q_ARG(QVariant, s));
}

void Game::setTimeMultiplier(int t) {
    timeMultiplier = t;
}