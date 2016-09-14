#ifndef TITA_H
#define TITA_H

#include <QMainWindow>
#include <QIODevice>
#include <QByteArray>
#include <QAudioOutput>
#include <QMap>

class Morse;


namespace Ui {
class Tita;
}

class Tita : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Tita(QWidget *parent = 0);
    ~Tita();
    
private slots:
    void on_bStartStop_clicked(bool checked);
	void on_bPause_clicked(bool checked);
	void processTextFinished();
	void processCharFinished(int, QChar, QString);

    void on_bTestSpeed_clicked();

    void on_cbLanguage_currentIndexChanged(int index);

    void on_hsFrequenz_valueChanged(int value);

    void on_hsPauseBtwWords_valueChanged(int value);

    void on_hsPauseBtwChars_valueChanged(int value);

    void on_hsBaseSpeed_valueChanged(int value);

private:
    Ui::Tita *ui;
	Morse *_morse;
};

class AudioOutput;

class Morse : public QObject
{
    Q_OBJECT

public:
    explicit Morse(QObject *parent = 0);
    ~Morse();

   QString toMorse( QChar );

public slots:
    void processText(QString text);
	void stop();
	void pause();
	void resume();
	void setCharPause(int);
	void setWordPause(int);
	void setBaseSpeed(int);
	void setFrequency(int);

private slots:
    void doKey();
	void nextChar();


signals:
	void textFinished();
	void charFinished(int, QChar, QString);

private:
	enum UserState {US_Play, US_Stop, US_Pause};
	UserState _state;
    QString _text;
    QString  _currentChar;
	int _currentPos;
	QString _lang;
	QMap< QString, QMap<QChar, QString> > _tables;
	AudioOutput *_tone;
	int _baseSpeed;
	int _charPause;
	int _wordPause;
	int _freq;



	
};



#endif // TITA_H
