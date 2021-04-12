﻿#include "SimulationObjects.h"
#include "SimulationWindow.h"
#include <qdebug.h>

extern QSqlDatabase s_DB;

QFile s_LogFile( "Log.txt" );
QDebug s_Debug( &s_LogFile );

void MessageOutput( QtMsgType type, const QMessageLogContext &context, const QString &msg )
  {
  QByteArray localMsg = msg.toLocal8Bit();
  static int LogCount = 0;
  if( ++LogCount >= 1000000 )
    return;
  switch( type )
    {
    case QtDebugMsg:
      s_LogFile.write( "Отладочное сообщение: " + localMsg + "\r\n" );
      break;
    case QtInfoMsg:
      s_LogFile.write( "Информация: " + localMsg + "\r\n" );
      break;
    case QtWarningMsg:
      s_LogFile.write( "Предупреждение: " + localMsg + "\r\n" );
      break;
    case QtCriticalMsg:
      s_LogFile.write( "Ошибка: " + localMsg + "\r\n" );
      break;
    case QtFatalMsg:
      s_LogFile.write( "Фатальная ошибка: " + localMsg + "\r\n" );
      abort();
    }
  s_LogFile.flush();
  }

uLong Random::m_A = 0;
uLong Random::m_C = 0;
double Random::m_Scale = 0.0;
uLong Random::m_Next = 0;

<<<<<<< HEAD
=======

>>>>>>> 74449217a985ddc26bacb84b61ecea85d6eb9b97
int main( int argc, char *argv[] )
  {
  s_LogFile.open( QIODevice::WriteOnly );
  qInstallMessageHandler( MessageOutput );
  QStringList paths = QCoreApplication::libraryPaths();
  paths.append( "." );
  paths.append( "imageformats" );
  paths.append( "platforms" );
  paths.append( "sqldrivers" );
  QCoreApplication::setLibraryPaths( paths );
  QApplication a( argc, argv );
  MainWindow Wnd;
  try
    {
    s_DB.setDatabaseName( "Production" );
    s_DB.setUserName("root");
    s_DB.setPassword("Sim1Production2");
    if( !s_DB.open() ) throw QString("База данных не открылась");
    ProductionState PS;
    Generator DetailParty; //создаем партии деталей
    Adder AllEquipment( &DetailParty, NULL );
    Switch SelectPath( &AllEquipment );
    Delay LieTime;
    AllEquipment.AddPrevBlock( &LieTime );
    Terminator Quit( &SelectPath );
    VectorQueues EquipmentQueue( &SelectPath, "Очереди к оборудованию " );
    for( int i = 1; i < 12; i++ )
      GroupEquipment *pEquipment = new GroupEquipment( i, &EquipmentQueue, &LieTime );
    Galvanic Galv( &SelectPath );
    AllEquipment.AddPrevBlock( &Galv );
    return a.exec();
    }
  catch( const QString& Msg )
    {
    QMessageBox::critical( nullptr, "Ошибка", Msg );
    }
  }

