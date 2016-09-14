#ifndef GAME_H
#define GAME_H
#include <QtQuick/QQuickItem>
#include <QtQuick/qsgnode.h>
#include <QtQuick/qsgflatcolormaterial.h>
#include "qcurver.h"
#include <QTimer>
#include <QTime>
#include <QKeyEvent>
#include <QObject>
#include "qcurver.h"
#include <QSGGeometry>
#include "curveitem.h"
#include "fastitem.h"
#define MAXPLAYERCOUNT 10
#define MAXITEMCOUNT 20


class Game : public QQuickItem
{
    Q_OBJECT
public:
    explicit Game(QQuickItem *parent = 0);
	Q_INVOKABLE void sendKey(Qt::Key k);
	Q_INVOKABLE void releaseKey(Qt::Key k);
	Q_INVOKABLE void start();
	Q_INVOKABLE void changeColor(int index, QColor color);
	Q_INVOKABLE void changeControls(int index, Qt::Key k, bool isRight);
	QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);
    ~Game();
	void addPlayer();
	short int playercount = 1;

signals:
	void stopItemTimer();
public slots:
	void progress();
	void curverDied(QCurver* who);
	void checkforIntersection(QPointF a, QPointF b);
private slots:
	void startNextRound();

private:
	QCurver* curver[MAXPLAYERCOUNT];
	bool alive[MAXPLAYERCOUNT];
	QColor colors[MAXPLAYERCOUNT];
	Qt::Key controls[MAXPLAYERCOUNT][2]; //first one is left key, second one is right key
	int score[MAXPLAYERCOUNT];
	CurveItem* items[MAXITEMCOUNT];
	QTimer* timer;
	QTimer* nextRoundTimer;
	QTime lastTime;
	QSGNode *node = 0;
	int roundCount = 1;
	void nextRound();
	int nextItemSpawn; //time in milliseconds
	QTime lastItemSpawn;
};

#endif // GAME_H