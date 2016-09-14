#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <math.h>

#include <QObject>
#include <QMainWindow>
#include <QIODevice>
#include <QTimer>
#include <QPushButton>
#include <QComboBox>
#include <QByteArray>

#include <QAudioOutput>

class Generator : public QIODevice
{
    Q_OBJECT
public:
    Generator(QObject *parent);
    ~Generator();

    void start();
    void stop();
	int generateMorseTone(QString seq, const QAudioFormat &format, int baseSpeed, int frequency);

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

private:
    void generateData(const QAudioFormat &format, qint64 durationUs, int frequency);

signals:
	void finished();

public:
	QByteArray m_buffer;
private:
    qint64 m_pos;
    //QByteArray m_buffer;
	QByteArray m_buffer1;
};

class AudioOutput : public QObject
{
    Q_OBJECT
public:
    AudioOutput(QObject*p);
    ~AudioOutput();

	void morse(QString, int baseSpeed=70, int freq=700);

signals:
	void finished();

private:
    void initializeAudio();
    //void createAudioOutput();

private:
    QTimer*          m_pullTimer;

    QAudioDeviceInfo m_device;
    Generator*       m_generator;
    QAudioOutput*    m_audioOutput;
    QIODevice*       m_output; // not owned
    QAudioFormat     m_format;

    bool             m_pullMode;
    //QByteArray       m_buffer;

private slots:
	void onFinished();
    void notified();
//    void pullTimerExpired();
//    void toggleMode();
    void toggleSuspendResume();
    void stateChanged(QAudio::State state);
    void deviceChanged(int index);
};

#endif

