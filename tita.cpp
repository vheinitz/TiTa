#include "tita.h"
#include "ui_tita.h"
#include <QTimer>

#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QtCore/qmath.h>
#include <QtCore/qendian.h>
#include "persistence.h"

const int DurationSeconds = 1;
const int ToneFrequencyHz = 600;
const int DataFrequencyHz = 44100;
const int BufferSize      = 32768;

#include <AudioOutput.h>

Tita::Tita(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Tita)
{
    ui->setupUi(this);
	
	_morse = new Morse(this);
	_morse->setBaseSpeed( ui->hsBaseSpeed->value() );
	_morse->setCharPause( ui->hsPauseBtwChars->value() );
	//_morse->setFrequency( ui->hsPauseBtwWords->value() );
	connect( _morse, SIGNAL(textFinished()), this, SLOT(processTextFinished()) );
	connect( _morse, SIGNAL(charFinished(int, QChar, QString)), this, SLOT(processCharFinished(int, QChar, QString)) );
	connect(ui->hsBaseSpeed, SIGNAL(valueChanged(int)), _morse, SLOT(setBaseSpeed(int)));
	connect(ui->hsPauseBtwChars, SIGNAL(valueChanged(int)), _morse, SLOT(setCharPause(int)));
	connect(ui->hsPauseBtwWords, SIGNAL(valueChanged(int)), _morse, SLOT(setWordPause(int)));
	connect(ui->hsFrequenz, SIGNAL(valueChanged(int)), _morse, SLOT(setFrequency(int)));
	PERSISTENCE_INIT( "heinitz-it.de", "Ti-Ta" );
	PERSISTENT("basespeed", ui->hsBaseSpeed, "value");
	PERSISTENT("pausechars", ui->hsPauseBtwChars, "value");
	PERSISTENT("pausewords", ui->hsPauseBtwWords, "value");
	PERSISTENT("frequenz", ui->hsFrequenz, "value");
	PERSISTENT("charsdigirsonly", ui->cbOnlyLettersAndDigits, "checked");
	PERSISTENT("lang", ui->cbLanguage, "currentIndex");
	PERSISTENT("text", ui->eText, "text");


}

Tita::~Tita()
{
    delete ui;
}

void Tita::on_bStartStop_clicked(bool checked)
{
    if (checked)
    {
        ui->bStartStop->setText(tr("Stop"));
		ui->eMorse->clear();
		_morse->processText( ui->eText->toPlainText() );
    }
    else
    {
        ui->bStartStop->setText(tr("Start"));
		ui->bPause->setText(tr("Pause"));
		_morse->stop();
    }
}

void Tita::on_bPause_clicked(bool checked)
{
    if (checked)
    {
        ui->bPause->setText(tr("Weiter"));
		_morse->pause();
    }
    else
    {
        ui->bPause->setText(tr("Pause"));
		_morse->resume();
    }
}

void Tita::processTextFinished()
{
	ui->bStartStop->setChecked(false);
	ui->bStartStop->setText(tr("Start"));
}

void Tita::processCharFinished(int idx, QChar ch, QString morse)
{
	ui->eMorse->insertPlainText( morse+"/"  );
}



Morse::Morse(QObject *parent):QObject(parent),_state(US_Stop),_baseSpeed(70),_charPause(2), _wordPause(6), _freq(700)
{

	_tone  = new AudioOutput(this);
	 
	 connect( _tone, SIGNAL(finished()), this, SLOT(doKey()) );

	_tables["international"]['A'] = ".-";
	_tables["international"]['B'] = "-...";
	_tables["international"]['C'] = "-.-.";
	_tables["international"]['D'] = "-..";
	_tables["international"]['E'] = ".";
	_tables["international"]['F'] = "..-.";
	_tables["international"]['G'] = "--.";
	_tables["international"]['H'] = "....";
	_tables["international"]['I'] = "..";
	_tables["international"]['J'] = ".---";
	_tables["international"]['K'] = "-.-";
	_tables["international"]['L'] = ".-..";
	_tables["international"]['M'] = "--";
	_tables["international"]['N'] = "-.";
	_tables["international"]['O'] = "---";
	_tables["international"]['P'] = ".--.";
	_tables["international"]['Q'] = "--.-";
	_tables["international"]['R'] = ".-.";
	_tables["international"]['S'] = "...";
	_tables["international"]['T'] = "-";
	_tables["international"]['U'] = "..-";
	_tables["international"]['V'] = "...-";
	_tables["international"]['W'] = ".--";
	_tables["international"]['X'] = "-..-";
	_tables["international"]['Y'] = "-.--";
	_tables["international"]['Z'] = "--..";

	_tables["international"]['Ä'] = ".-.-";
	_tables["international"]['Á'] = ".--.-";
	_tables["international"]['Å'] = ".--.-";
	_tables["international"]['É'] = "..-..";
	_tables["international"]['Ñ'] = "--.--";
	_tables["international"]['Ö'] = "---.";
	_tables["international"]['Ü'] = "..--";

	_tables["international"]['0'] = "-----";
	_tables["international"]['1'] = ".----";
	_tables["international"]['2'] = "..---";
	_tables["international"]['3'] = "...--";
	_tables["international"]['4'] = "....-";
	_tables["international"]['5'] = ".....";
	_tables["international"]['6'] = "-....";
	_tables["international"]['7'] = "--...";
	_tables["international"]['8'] = "---..";
	_tables["international"]['9'] = "----.";

	_tables["international"]['.'] = ".-.-.-";

	_tables["international"]['.'] = ".-.-.-";
	_tables["international"][','] = "--..--";
	_tables["international"][':'] = "---...";
	_tables["international"]['?'] = "..--..";
	_tables["international"]['\''] = ".----.";
	_tables["international"]['/'] = "-..-.";
	_tables["international"]['('] = "-.--.-";
	_tables["international"][')'] = "-.--.-";

	_tables["international"]['"'] = ".-..-.";
	_tables["international"]['@'] = ".--.-.";
	_tables["international"]['=)'] = "-...-";


	_lang = "international";

}

Morse::~Morse()
{
}



void Morse::processText(QString text)
{
    _text = text.simplified();
    _currentPos=0;
	_state = US_Play;
    doKey();

}

void Morse::stop()
{
	_text.clear();
    _currentPos=0;
    _state = US_Stop;

}

void Morse::pause()
{
    _state = US_Pause;
}

void Morse::resume()
{
    _state = US_Play;
    doKey();

}

void Morse::setCharPause(int p)
{
	_charPause = p;
}

void Morse::setWordPause(int p )
{
	_wordPause = p;
}

void Morse::setBaseSpeed(int s)
{
	_baseSpeed = s;
}

void Morse::setFrequency(int f)
{
	_freq = f;
}


void Morse::nextChar()
{
	_tone->morse( _currentChar, _baseSpeed, _freq );	
}

void Morse::doKey()
{
	if ( _state != US_Play )
	{
        return;
	}

    if ( _currentPos >= _text.size() )
	{
		emit textFinished();
        return;
	}

	QChar ch = _text.at(_currentPos).toUpper();
	_currentChar = toMorse(ch);
    _currentPos++;

	if ( _currentChar!="#" )
		QTimer::singleShot( _charPause * _baseSpeed, this , SLOT(nextChar()) );

	else if ( _currentChar=="#" )
		QTimer::singleShot( _wordPause * _baseSpeed, this , SLOT(nextChar()) );


	//_tone->morse( currentChar, ui->hsBaseSpeed->value() );
	emit charFinished(_currentPos, ch, _currentChar);
}

QString Morse::toMorse( QChar c )
{
	if ( _tables.contains(_lang) && _tables[_lang].contains(c) )
		return _tables[_lang][c];
	return "#";
}

void Tita::on_bTestSpeed_clicked()
{
	_morse->stop();
	_morse->processText("SOS SOS SOS");
}

void Tita::on_cbLanguage_currentIndexChanged(int index)
{

}

void Tita::on_hsFrequenz_valueChanged(int value)
{
    ui->eFrequenz->setText( QString::number(value) );
}

void Tita::on_hsPauseBtwWords_valueChanged(int value)
{
    ui->ePauseBtwWords->setText( QString::number(value) );
}

void Tita::on_hsPauseBtwChars_valueChanged(int value)
{
    ui->ePauseBtwChars->setText( QString::number(value) );
}

void Tita::on_hsBaseSpeed_valueChanged(int value)
{
    ui->eBaseSpeed->setText( QString::number(value) );
}
