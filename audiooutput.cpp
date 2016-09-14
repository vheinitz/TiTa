
#include <QDebug>
#include <QVBoxLayout>

#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QtCore/qmath.h>
#include <QtCore/qendian.h>
#include <QApplication>
#include "audiooutput.h"

/*const QString AudioOutput::PushModeLabel(tr("Enable push mode"));
const QString AudioOutput::PullModeLabel(tr("Enable pull mode"));
const QString AudioOutput::SuspendLabel(tr("Suspend playback"));
const QString AudioOutput::ResumeLabel(tr("Resume playback"));
*/
const int DurationSeconds = 1;
const int ToneFrequencyHz = 600;
const int DataFrequencyHz = 44100;
const int BufferSize      = 32768;


Generator::Generator(QObject *parent)
    :   QIODevice(parent)
    ,   m_pos(0)
{
	//generateMorseTone(".--.");
}

int Generator::generateMorseTone(QString seq, const QAudioFormat &format, int baseSpeed, int frequency)
{

	int ti=baseSpeed*100;
	int ta=ti*3;
	int lenus=0;

	m_buffer1.clear();
	m_buffer.clear();
	for(int i=0; i<seq.size(); ++i)
	{
		QChar c = seq.at(i);
		if (c == '_' || c == '-')
		{
			generateData(format, ta, frequency);
			m_buffer1 += m_buffer;
			generateData(format, ti, 0);
			m_buffer1 += m_buffer;
			lenus+=ta+ti;
		}
		else if (c == '.')
		{
			generateData(format, ti, frequency);
			m_buffer1 += m_buffer;
			generateData(format, ti, 0);
			m_buffer1 += m_buffer;
			lenus+=ti+ti;

		}
		/*else if (c == '#')
		{
			generateData(format, ti*6, 0);
			m_buffer1 += m_buffer;
			lenus+=ti*6;
		}*/
		else
			;//TODO ERROR
		
	}

	m_buffer = m_buffer1;
	return lenus;
}

Generator::~Generator()
{

}

void Generator::start()
{
    open(QIODevice::ReadOnly);
}

void Generator::stop()
{
    m_pos = 0;
    close();
}

void Generator::generateData(const QAudioFormat &format, qint64 durationUs, int frequency)
{
    const int channelBytes = format.sampleSize() / 8;
    const int sampleBytes = format.channels() * channelBytes;

    qint64 length = (format.frequency() * format.channels() * (format.sampleSize() / 8))
                        * durationUs / 100000;

 //   Q_ASSERT(length % sampleBytes == 0);
	length -= length % sampleBytes;
    Q_UNUSED(sampleBytes) // suppress warning in release builds

    m_buffer.resize(length);
    unsigned char *ptr = reinterpret_cast<unsigned char *>(m_buffer.data());
    int sampleIndex = 0;

    while (length) {
        const qreal x = qSin(2 * M_PI * frequency * qreal(sampleIndex % format.frequency()) / format.frequency());
        for (int i=0; i<format.channels(); ++i) {
            if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) {
                const quint8 value = static_cast<quint8>((1.0 + x) / 2 * 255);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::SignedInt) {
                const qint8 value = static_cast<qint8>(x * 127);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::UnSignedInt) {
                quint16 value = static_cast<quint16>((1.0 + x) / 2 * 65535);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<quint16>(value, ptr);
                else
                    qToBigEndian<quint16>(value, ptr);
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
                qint16 value = static_cast<qint16>(x * 32767);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<qint16>(value, ptr);
                else
                    qToBigEndian<qint16>(value, ptr);
            }

            ptr += channelBytes;
            length -= channelBytes;
        }
        ++sampleIndex;
    }
}

qint64 Generator::readData(char *data, qint64 len)
{
	static qint64 bufread=0;
	//if (bufread >= m_buffer.length() - len )
	//	return 0;

    qint64 total = 0;
    while (len - total > 0) {
        const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
		if ( (m_pos + chunk) % m_buffer.size() == 0 )
		{
			stop();
			emit finished();			
			return 0;
		}
        memcpy(data + total, m_buffer.constData() + m_pos, chunk);
        m_pos = (m_pos + chunk) % m_buffer.size();
        total += chunk;
    }
	bufread += len;
    return total;	
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 Generator::bytesAvailable() const
{
    return m_buffer.size() + QIODevice::bytesAvailable();
}

AudioOutput::AudioOutput(QObject *p)
    :   QObject(p)
	,	m_pullTimer(new QTimer(this))
    ,   m_device(QAudioDeviceInfo::defaultOutputDevice())
    ,   m_generator(0)
    ,   m_audioOutput(0)
   // ,   m_output(0)
   // ,   m_buffer(BufferSize, 0)
{
    initializeAudio();
}

void AudioOutput::onFinished()
{
	emit finished();
}

void AudioOutput::initializeAudio()
{
   // connect(m_pullTimer, SIGNAL(timeout()), SLOT(pullTimerExpired()));

    m_pullMode = true;

    m_format.setFrequency(DataFrequencyHz);
    m_format.setChannels(1);
    m_format.setSampleSize(16);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(m_format)) {
        qWarning() << "Default format not supported - trying to use nearest";
        m_format = info.nearestFormat(m_format);
    }

    m_generator = new Generator(this);
	//connect( m_generator, SIGNAL(finished()), this, SLOT(onFinished()) );
	m_audioOutput = new QAudioOutput(m_device, m_format, this);
	//int len = m_generator->generateMorseTone( "...---...", m_format, ToneFrequencyHz );


	

    //createAudioOutput();
	
}

void AudioOutput::morse(QString m, int baseSpeed, int freq)
{
	m_generator->generateMorseTone( m, m_format,baseSpeed, freq );
	//m_generator->start();
    m_output = m_audioOutput->start();//(m_generator);
	if (m_output)
	{
		qint64 bw = m_output->write(m_generator->m_buffer);

		while ( bw < m_generator->m_buffer.size() )
		{
			bw += m_output->write(m_generator->m_buffer.data()+bw, m_generator->m_buffer.size() - bw);
			QApplication::processEvents();
		}
		//QTimer::singleShot(50, this, SLOT( onFinished() ) );
	}
	emit finished();
	
}


/*void AudioOutput::createAudioOutput()
{
    delete m_audioOutput;
    m_audioOutput = 0;
    
    connect(m_audioOutput, SIGNAL(notify()), SLOT(notified()));
    
}*/

AudioOutput::~AudioOutput()
{

}


void AudioOutput::deviceChanged(int index)
{
//    createAudioOutput();
}

void AudioOutput::notified()
{
 
}

/*void AudioOutput::pullTimerExpired()
{
    if (m_audioOutput && m_audioOutput->state() != QAudio::StoppedState) {
        int chunks = m_audioOutput->bytesFree()/m_audioOutput->periodSize();
        while (chunks) {
           const qint64 len = m_generator->read(m_buffer.data(), m_audioOutput->periodSize());
           if (len)
               m_output->write(m_buffer.data(), len);
           if (len != m_audioOutput->periodSize())
               break;
           --chunks;
        }
    }
}*/

/*void AudioOutput::toggleMode()
{
    m_pullTimer->stop();
    m_audioOutput->stop();

    if (m_pullMode) {
        m_output = m_audioOutput->start();
        m_pullMode = false;
        m_pullTimer->start(20);
    } else {
        m_pullMode = true;
        m_audioOutput->start(m_generator);
    }
}*/

void AudioOutput::toggleSuspendResume()
{
    if (m_audioOutput->state() == QAudio::SuspendedState) {
        qWarning() << "status: Suspended, resume()";
        m_audioOutput->resume();
    } else if (m_audioOutput->state() == QAudio::ActiveState) {
        qWarning() << "status: Active, suspend()";
        m_audioOutput->suspend();
    } else if (m_audioOutput->state() == QAudio::StoppedState) {
        qWarning() << "status: Stopped, resume()";
        m_audioOutput->resume();
    } else if (m_audioOutput->state() == QAudio::IdleState) {
        qWarning() << "status: IdleState";
    }
}

void AudioOutput::stateChanged(QAudio::State state)
{
    qWarning() << "state = " << state;
}
