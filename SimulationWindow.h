#ifndef MAINWINDOW
#define MAINWINDOW
#include <qapplication.h>
#include <QMainWindow>
#include <qtextcodec.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qinputdialog.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <QSpinBox>
#include <QScrollArea>
#include <QSqlQuery>
#include <QTableView>
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QDialog>
#include <QTableWidget>

class MainWindow : public QMainWindow 
{
  Q_OBJECT

  QAction *m_pTableNine;
  QAction *m_pTableTen;
  QAction *m_pMachineChartOne;
  QAction *m_pMachineChartTwo;
  QAction *m_pMachineChartFour;
  QAction *m_pModeling;
public:
  MainWindow ();
  public slots:
  void slotOpenDialog ();
  void slotOpenSumulation ();
  void slotShowTableNine ();
  void slotShowTableTen ();
  void slotShowMachineChartOne ();
  void slotShowMachineChartTwo ();
  void slotShowMachineChartFour ();
};

class DlgInputParms : public QDialog
{
  Q_OBJECT
    QDoubleSpinBox *m_pShiftLength;
  QSpinBox *m_pShiftCount;
  QDoubleSpinBox *m_pStartDay;
  QSpinBox *m_pTimeSimulation;
  QSpinBox *m_pVariant;
public:
  DlgInputParms ();
  public slots:
  void accept ();
};
#endif
