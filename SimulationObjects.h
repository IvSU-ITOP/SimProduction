#include "RandomObjects.h"
#include <qtextcodec.h>
#include <QSet>
#include <QMap>
#include <QVector>
#include <QDoubleSpinBox>
#include <QDialog>
#include <qqueue.h>
#include <QSqlDatabase>
#include <QMainWindow>
#include <qmenu.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <QSqlQueryModel>
#include <qfile.h>
#include <qprogressbar.h>
#include <qthread.h>
#include <qtimer.h>

int Round( double Value );
double Round( double Value, int Digits );

class Resource;
class VectorQueues;
enum ServiceResult { Success, IsBusy, BadActor };
enum BenchSate { bnFree, bnBusy, bnRepair, bnEndShift };

struct Step
  {
  int m_GroupId;
  double m_DetailTime;
  double m_PartyTime;
  double m_LieTime;
  double m_Galvanic;
  Step() {};
  Step(int IGroup, double IDetail, double IParty, double LieTime, double Galvanic) : m_GroupId(IGroup), 
    m_DetailTime(IDetail), m_PartyTime(IParty), m_LieTime(LieTime), m_Galvanic(Galvanic) {}
  };

class Time
  {
  friend class DlgInputParms;
  friend class Simulator;
  friend class ProductionState;
  friend class Route;
  friend class GroupEquipment;
  friend class NamedBlock;
  friend class Bench;
  static double sm_ShiftLength;
  static int sm_ShiftCount;
  static double sm_ReShift; // время пересменки 
  static double sm_Precision;
  double  m_Hour;
  static Time sm_CurrentTime;
  public:
    static const double sc_BadTime;
    static double sm_StartWork;
    static const Time& GetCurrentTime() { return sm_CurrentTime; }
    static bool SetDayParameters(double ShiftLength, int ShiftCount, double StartDay);
    static int ShiftCount() { return sm_ShiftCount; }
    static double DobeLength() { return sm_ShiftLength * sm_ShiftCount; }
    Time(int Day) : m_Hour(Day * 24.0) {} //переписать с контролем ошибки.
    Time(double Hour);
    Time(const Time& T) : m_Hour(T.m_Hour) {}          //Реализовать можно прямо здесь.
    Time() : m_Hour(sc_BadTime) {}
    Time EndCurrentShift() const;  //Время конца текущей смены. Т.е. исходный момент времени не произвольный, а должен быть в смене как в замкнутом интервале. 
    Time NextShift() const; //Время начала следующей смены
    int GetCurrentShift() const;//Номер текущей смены
    Time& operator = (const Time&);
    Time& operator += (double);
    Time operator + (double) const;
    double operator - (const Time&) const;
    bool operator < (const Time&) const;
    bool operator > (const Time&) const;
    bool operator == (const Time&) const;
    bool operator != (const Time&) const;
    bool IsNoTime() const { return m_Hour == sc_BadTime; }
    void SetNoTime() { m_Hour = sc_BadTime; }
    double Hour() const { return m_Hour;  }
    double DobeLength (double Hour) { return Hour / sm_ShiftLength*sm_ShiftCount; }
  }; 

class Route : public QVector < Step >
  {
  friend class Generator;
  friend class GroupEquipment;
  friend class ProductionState;
  int m_DetailType;
  int m_Applicability;
  int m_ProductId;
  int m_ReadyCount;
  int m_PlanCount;
  int m_PartyCount;
  int m_PartyStarted;
  int m_OldVolumeParty;
  int m_VolumeParty;
  double m_PrelVolumeParty;
  double m_StartInterval;
  double m_SumReadjustmentTime;
  double m_SumDetailTime;
  double m_SumGalvanic;
  double m_TotalTime;
  double m_SumLieTime;
  QString m_DetailName;
  Time m_TimeStartParty; //Момент следующего запуска партии деталей
  Time m_CorrectedTimeStartParty; //Момент следующего запуска партии деталей
  double CalcIncomplete( int ProdCount );
  public:
    static int sm_Variant;
    Route(int Id, const QString& DetailName);
    double PlanPercent() { return m_ReadyCount * 100.0 / m_PlanCount; }
    Route() {}
    int DetailType() { return m_DetailType; }
    QString DetailName() { return m_DetailName; }
    class Actor* GetParty();
    double BusyTime( int GroupId );
    void StartWork();
    double PrelVolumeParty () { return m_PrelVolumeParty; }
    double VolumeParty () { return m_VolumeParty; }
    double OldVolumeParty() { return m_OldVolumeParty; }
    double SumReadjustmentTime() { return m_SumReadjustmentTime; }
    double SumDetailTime () { return m_SumDetailTime; }
    double SumGalvanic () { return m_SumGalvanic; }
    double SumLieTime () { return m_SumLieTime; }
    int ProductFinished() { return m_ReadyCount / m_Applicability; }
    void Finish() { m_ReadyCount += m_VolumeParty; }
    int ReadyCount () { return  m_ReadyCount; }
    int PlanCount () { return m_PlanCount; }
    int PartyCount() { return m_PartyCount; }
    int ProductId() { return m_ProductId; }
    int Applicability() { return m_Applicability; }
  };

class Actor // Партия деталей.
  {
  friend class GroupEquipment;
  Time m_TimeStart;
  Time m_NextEventTime;
  Route* m_pRoute;
  int m_CurrentStep;
  int m_DetailCount;
  RNormal m_DetailWorkTime;
  RNormal m_PartyWorkTime;
  int m_RawCount; //число необработанных деталей
  Time m_StartQueue;
  double m_LieTime;
  public:
    Actor(Time TimeStart, int DetailCount, Route *pRouter);
    int DetailType() { return m_pRoute->DetailType(); }
    double PlanPercent() { return m_pRoute->PlanPercent(); }
    const Step& CuttentStep() { return (*m_pRoute)[m_CurrentStep]; }
    ServiceResult SelectQueue(VectorQueues*);
    bool NextStep();
    bool GetRawDetail();
    double DetailWorkTime();
    int GetCurrentGroupEquipment();
    double GetGalvanicTime();
    double GetLieTime();
    int GetId() const { return m_pRoute->DetailType(); }
    void Finish() { m_pRoute->Finish(); }
  };

class BaseActingBlock;
class Simulator
  {
  friend class DlgInputParms;
  friend class Route;
  protected:
  int m_CurrentActor;
  static Time sm_End;
  public:
    vector<BaseActingBlock*> m_ActingBlocks;
    Simulator();
    void AddBlock(BaseActingBlock* pBlock) { m_ActingBlocks.push_back(pBlock); }
    virtual void Evaluate();
  };

typedef Actor DetailParty;

class BaseActingBlock
  {
  protected:
    Time m_NextEventTime;
    static const double sc_NoTime;
    RNumber *m_pRNumber;
    virtual double GetRNumber(Actor *pActor = NULL) { return m_pRNumber->GetNumber(); }
  public:
    BaseActingBlock() : m_pRNumber(NULL)
      {
      sm_pSimulator->AddBlock(this);
      }
    static Simulator *sm_pSimulator;
    virtual void Evaluate() {};
    virtual Time NextEventTime() { return m_NextEventTime; };
    virtual void Report() {}
    virtual QString GetName() { return ""; }
    virtual void StartWork() {};
  };

class ActingBlock : public BaseActingBlock
  {
  friend Simulator;
  protected:
    ActingBlock *m_pNextBlock;
  public:
    ActingBlock() : m_pNextBlock( NULL )
      {}
    virtual ServiceResult Evaluate( Actor* ) { return IsBusy; }
    virtual void SetNextBlock( ActingBlock *pNextBlock ) { m_pNextBlock = pNextBlock; }
  };

class NamedBlock : public ActingBlock
  {
  protected:
  double m_WorkTime;
  Time m_TimeStart;
  QString m_Name;
  public:
    NamedBlock() {}
    NamedBlock(QString Name) : m_Name(Name), m_WorkTime(Time::sm_StartWork), m_TimeStart(Time::sm_StartWork) {};
    NamedBlock(QString Name, RNumber *pRNumber) : m_Name(Name), m_WorkTime(0.0), m_TimeStart(Time::sm_StartWork) { m_pRNumber = pRNumber; };
    virtual void Report();
    QString GetName() { return m_Name; }
    virtual void StartWork();
    QString Name () { return m_Name; }
  };

class Queue : public NamedBlock
  {
  vector<Actor*> m_Actors;
  size_t m_MaxLen;
  double m_QueueTime;
  public:
    Queue( QString Name, ActingBlock *pPrevBlock ) : NamedBlock( Name ), m_MaxLen( 0 ) {}
    virtual ServiceResult Evaluate(Actor *);
    virtual void Report();
    virtual Actor* GetActor();
    size_t GetLen() { return m_Actors.size(); }
    const Actor& operator[] ( int i ) { return *m_Actors[i]; }
  };

class VectorQueues;

class Service : public NamedBlock
  {
  protected:
    Actor *m_pCurrentActor;
    Queue *m_pPrevQueue;
  public:
    Service() : m_pPrevQueue( nullptr ), m_pCurrentActor(nullptr) {}
    Service( QString Name, ActingBlock *pPrevQueue, RNumber * pRNumber ) : 
      NamedBlock( Name, pRNumber ), m_pPrevQueue( (Queue *) pPrevQueue ), m_pCurrentActor(NULL)
      { pPrevQueue->SetNextBlock( this ); }
      //Service( string Name, VectorQueues *pQueues, RNumber * pRNumber );
    //его вызывает симулятор без параметра. В тот момент, когда сервис закончил обработку очередного
    //актора. Берём текущего актора и предаем дальше по цепочке. Если в очереди никого нет,
    // время работы увеличиваем. Добавляем к своему времени время до события. 
    virtual ServiceResult Evaluate(Actor *);
    //эта ф-я вызывается предыдущим блоком. Актор передается из предыдущего блока. 
    //Делается попытка передачи актора.
    //Если текущее время события живое, то сервис занят. 
    void SetQueue( Queue *pQueue ) { m_pPrevQueue = pQueue; pQueue->SetNextBlock(this); }
    //Объект попадает в очередь
  };

class VectorQueues : public ActingBlock, public vector<Queue*>
  {
  QString m_Prefix;
  public:
    VectorQueues( ActingBlock *pPrevBlock, QString Prefix  ) : m_Prefix( Prefix ) { pPrevBlock->SetNextBlock(this); }
    virtual void SetNextBlock( ActingBlock* );
    virtual ServiceResult Evaluate(Actor *pActor ) { return pActor->SelectQueue(this); } 
    ~VectorQueues();
  };

class Switch : public ActingBlock
  {
  vector<ActingBlock*> m_NextBlocks;
  public:
    Switch( ActingBlock *pPrevBlock ) { pPrevBlock->SetNextBlock(this); }
    virtual void SetNextBlock( ActingBlock *pPrevBlock ) { m_NextBlocks.push_back( pPrevBlock ); }
    virtual ServiceResult Evaluate(Actor *);
  };

class Worker
  {
  QString m_Name;  //Название профессии
  double m_PropUnappear;
  int m_ListCount;
  static Random sm_TestAppear;
  public:
    Worker() {}
    Worker( const QString& Name, double PropUnappear, int ListCount ) :
      m_Name( Name ), m_PropUnappear( PropUnappear ), m_ListCount( ListCount ) {}
    int ListCount() { return m_ListCount; }
    int AppearCount();
    const QString& Name() { return m_Name; }
  };

class GroupEquipment;

struct Incomplete
  {
  int m_Party;
  int m_Detail;
  public:
    Incomplete() : m_Party( 0 ), m_Detail( 0 ) {}
    Incomplete( int Party, int Detail ) : m_Party( Party ), m_Detail( Detail ) {}
  };

class IncompleteMap : public QMap<int, Incomplete>
  {
  public:
    void Add(int Id, int Party, int Detail );
  };

struct Product
  {
  int m_Id;
  QString m_Name;
  double m_Price;
  int m_Plan;
  int m_Finished;
  Product() {}
  Product( int Id, const QString& Name, double Price, int Plan ) : m_Id( Id ), m_Name( Name ), 
    m_Price( Price ), m_Plan( Plan ), m_Finished( m_Plan * 1000 ) {}
  void Finish() { if( m_Finished == m_Plan * 1000 ) m_Finished = 0; }
  };

class WaitWindow : public QDialog
  {
  public:
    WaitWindow();
    int exec();
  };

class ProductionState : public QVector<Route>, public Simulator
  {
  bool m_ShiftStarted;
  Time m_EndWorkTime;
  WaitWindow *m_pWait;
  public:
  static ProductionState* sm_pProductionState;
  static int sm_WorkDayCount;
  IncompleteMap m_IncompleteProduction, m_IncompleteProductionOld;
  QVector<Product> m_Ware;
  ProductionState();
  void Finish( DetailParty* pParty );
  void RemoveParty(DetailParty*) {}
  void StartShift() { m_ShiftStarted = true; }
  void EndShift();
  bool ShiftStarted() { return m_ShiftStarted; }
  void Evaluate();
  Time GetEndWorkTime() { return m_EndWorkTime; }
  };

class Generator : public ActingBlock
  {
  Route *m_pNextRoute;
  public:
    Generator() : m_pNextRoute(nullptr) {}
    void Evaluate();
    virtual void StartWork();
  };

class Bench : public BaseActingBlock
  {
  friend GroupEquipment;
  friend ProductionState;
  enum StateType { bnchRepair, bnchWork, bnchNoWorker, bnchReadjust, bnchFree }; //состояние станка
  QMap <StateType, double> m_StateQueue;
  GroupEquipment *m_pGroup;
  int m_Id;
  double m_ReadjustmentTime;
  double m_TotalReadjustmentTime;
  double m_RepairTime; //Время, необходимое для завершения профилактики.  Если профилактика не идет, то 0
  double m_TotalRepairTime; //Сумма всего времени профилактики (для отчета)
  double m_TimeNoWorker;    //Время простоя из-за отсутствия рабочего (для отчета)
  double m_TotalWorkTime;  //Сумма всего всего времени, затраченного на обработку деталей (для отчета)
  double m_NextRepairTime; //Плановое время начала очередной профилактики. Уменьшается после обработки каждой детали
  double m_WorkTime; //Текущее рабочее время
  Time m_StartFree;
  double m_TotalFreeTime;
  StateType m_State;
  void StartFreeTime();
  void EndFreeTime();
  void CorrectRepairTime();
  void SetReadjustment(double);
  public:
    Bench();
    void SetWorker(bool IsWorker);
    virtual void Evaluate();
    bool IsRepair();
    bool SetDetail();
    QString GetName();
    void Report();
  };

class GroupEquipment : public Service
  {
  friend Bench;
  friend class ProductionState;
  enum EventType { evSchiftStart, evSchiftEnd };
  QMap <Time, EventType> m_EventQueue;
  int m_GroupId;
  QVector<Bench> m_BenchArray;
  RNormal m_RepairTimeNeed;
  double m_RepairInterval;
  double m_AnotherTime; 
  Worker m_Worker;
  double m_MeanRepairTime;
  double m_MeanCoeff;
  double m_TotalWorkFund;
  double m_TotalReadjustmentTime;
  double m_TotalRepairTime;
  double m_TimeNoWorker;    //Время простоя из-за отсутствия рабочего (для отчета)
  double m_TotalWorkTime;  //Сумма всего всего времени, затраченного на обработку деталей (для отчета)
  double m_TotalFreeTime;
  void BackDetail() { m_pCurrentActor->m_RawCount++; }
  void StartFree();
  void NewParty();
  public:
    GroupEquipment(int GroupId, ActingBlock *pPrevQueue, class Delay* );
    ServiceResult Evaluate(DetailParty* pParty);
    void Evaluate();
    double RepairTime();
    double NoWorkerTime() { return 0.0;  }
    double WorkTime();
    double ReadjustmentTime();  //Время переналадки;
    Bench::StateType GetDetail( Bench &Bench );
    Time NextEventTime();
    bool IsTimeOff();
    virtual void StartWork();
    virtual void Report();
    void ShiftsReport();

    double TotalWorkFund () { return m_TotalWorkFund; }
    double TotalRepairTime ()  { return m_TotalRepairTime; }
    double TimeNoWorker ()  { return m_TimeNoWorker; }
    double TotalFreeTime () { return m_TotalFreeTime; }
    double TotalReadjustmentTime () { return m_TotalReadjustmentTime; } 
    double TotalWorkTime () { return m_TotalWorkTime; }
  };

class Delay : public NamedBlock
  {
  protected:
    QMap<Time, DetailParty *> m_PartyArray;
  public:
    Delay();
    virtual ServiceResult Evaluate( Actor * );
    virtual void Evaluate();
    virtual void StartWork();
    virtual void Report();
  };

class Galvanic : public Delay
  {
  protected:
    int m_GroupId;
  public:
    Galvanic( ActingBlock *pPrevBlock );
    ServiceResult Evaluate(DetailParty* pParty);
    virtual void Evaluate();
  };

class Terminator : public ActingBlock
  {
  public:
    Terminator(ActingBlock *pPrevBlock) { pPrevBlock->SetNextBlock(this); }
    ServiceResult Evaluate(Actor *pActor);
    void Report();
  };

class Adder : public ActingBlock
  {
  public:
    Adder(ActingBlock *pPrevBlock = NULL, ...);
    virtual ServiceResult Evaluate(Actor *pActor) { return m_pNextBlock->Evaluate(pActor); }
    void AddPrevBlock(ActingBlock *pPrevBlock) { pPrevBlock->SetNextBlock(this); }
  };

