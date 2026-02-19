/*
状态机学习，包含3-5个状态
事物：学生
状态：起床、上学、吃饭、写作业、睡觉
切换状态通过执行相应的事件进行转移

night8针对此状态机的修改：增加超时与错误态
*/
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

using namespace std;

//状态项
class FSMItem {
    friend class FSM;

private:
    static void getup()       { cout << "you should get up!!" << endl; }
    static void go_school()   { cout << "you should go school!!" << endl; }
    static void eat()         { cout << "you should eat!!" << endl; }
    static void do_homework() { cout << "you should do homework!!" << endl; }
    static void go_sleep()    { cout << "you should go sleep!!" << endl; }

    // 新增：超时/错误动作
    static void on_timeout()  { cout << "[TIMEOUT] state timeout happened!" << endl; }
    static void on_error()    { cout << "[ERROR] invalid event for current state!" << endl; }

public:
    enum class State {
        GETUP = 0,
        GO_SCHOOL,
        EAT,
        DO_HOMEWORK,
        GO_SLEEP,

        // 新增：超时态/错误态
        TIMEOUT,
        ERROR
    };

    enum class Events {
        EVENT1 = 0,
        EVENT2,
        EVENT3,

        // 新增：超时事件（由 tick() 自动触发）
        EVENT_TIMEOUT
    };

    FSMItem(State curState, Events event, void(*action)(), State nextState)
        : _curState(curState), _event(event), _action(action), _nextState(nextState) {}

private:
    State _curState;      // 现态
    Events _event;        // 条件
    void(*_action)();     // 动作
    State _nextState;     // 次态
};

class FSM {
private:
    vector<FSMItem*> _fsmTable;

    // 记录状态进入时间，用于超时判定
    std::chrono::steady_clock::time_point _enterTime;

    // 可选：记录上一次状态（你未来想做“回退/重试”时有用）
    FSMItem::State _lastState;

    static const char* stateName(FSMItem::State s) {
        switch (s) {
        case FSMItem::State::GETUP:        return "GETUP";
        case FSMItem::State::GO_SCHOOL:    return "GO_SCHOOL";
        case FSMItem::State::EAT:          return "EAT";
        case FSMItem::State::DO_HOMEWORK:  return "DO_HOMEWORK";
        case FSMItem::State::GO_SLEEP:     return "GO_SLEEP";
        case FSMItem::State::TIMEOUT:      return "TIMEOUT";
        case FSMItem::State::ERROR:        return "ERROR";
        default:                           return "UNKNOWN";
        }
    }

    // 每个状态允许停留的最大时间（你可以按需调整）
    static std::chrono::milliseconds timeoutOf(FSMItem::State s) {
        using namespace std::chrono;
        switch (s) {
        case FSMItem::State::GETUP:        return 1500ms;
        case FSMItem::State::GO_SCHOOL:    return 1200ms;
        case FSMItem::State::EAT:          return 1000ms;
        case FSMItem::State::DO_HOMEWORK:  return 2000ms;
        case FSMItem::State::GO_SLEEP:     return 1500ms;
        // TIMEOUT/ERROR 不做超时（也可以做：超过一段时间自动回到 GETUP）
        case FSMItem::State::TIMEOUT:      return 0ms;
        case FSMItem::State::ERROR:        return 0ms;
        default:                           return 0ms;
        }
    }

    void initFSMTable() {
        // 正常流程
        _fsmTable.push_back(new FSMItem(FSMItem::State::GETUP,       FSMItem::Events::EVENT1, &FSMItem::getup,       FSMItem::State::GO_SCHOOL));
        _fsmTable.push_back(new FSMItem(FSMItem::State::GO_SCHOOL,   FSMItem::Events::EVENT2, &FSMItem::go_school,   FSMItem::State::EAT));
        _fsmTable.push_back(new FSMItem(FSMItem::State::EAT,         FSMItem::Events::EVENT3, &FSMItem::eat,         FSMItem::State::DO_HOMEWORK));
        _fsmTable.push_back(new FSMItem(FSMItem::State::DO_HOMEWORK, FSMItem::Events::EVENT1, &FSMItem::do_homework, FSMItem::State::GO_SLEEP));
        _fsmTable.push_back(new FSMItem(FSMItem::State::GO_SLEEP,    FSMItem::Events::EVENT2, &FSMItem::go_sleep,    FSMItem::State::GETUP));

        // 新增：任何正常态超时 -> TIMEOUT
        _fsmTable.push_back(new FSMItem(FSMItem::State::GETUP,       FSMItem::Events::EVENT_TIMEOUT, &FSMItem::on_timeout, FSMItem::State::TIMEOUT));
        _fsmTable.push_back(new FSMItem(FSMItem::State::GO_SCHOOL,   FSMItem::Events::EVENT_TIMEOUT, &FSMItem::on_timeout, FSMItem::State::TIMEOUT));
        _fsmTable.push_back(new FSMItem(FSMItem::State::EAT,         FSMItem::Events::EVENT_TIMEOUT, &FSMItem::on_timeout, FSMItem::State::TIMEOUT));
        _fsmTable.push_back(new FSMItem(FSMItem::State::DO_HOMEWORK, FSMItem::Events::EVENT_TIMEOUT, &FSMItem::on_timeout, FSMItem::State::TIMEOUT));
        _fsmTable.push_back(new FSMItem(FSMItem::State::GO_SLEEP,    FSMItem::Events::EVENT_TIMEOUT, &FSMItem::on_timeout, FSMItem::State::TIMEOUT));

        // 新增：TIMEOUT 态的恢复策略（这里简单设置：EVENT1 重置回 GETUP；EVENT3 进入 ERROR）
        _fsmTable.push_back(new FSMItem(FSMItem::State::TIMEOUT, FSMItem::Events::EVENT1, &FSMItem::getup,     FSMItem::State::GETUP));
        _fsmTable.push_back(new FSMItem(FSMItem::State::TIMEOUT, FSMItem::Events::EVENT3, &FSMItem::on_error,  FSMItem::State::ERROR));

        // 新增：ERROR 态恢复策略（EVENT1 重置回 GETUP）
        _fsmTable.push_back(new FSMItem(FSMItem::State::ERROR, FSMItem::Events::EVENT1, &FSMItem::getup, FSMItem::State::GETUP));
    }

public:
    FSMItem::State _curState;

public:
    FSM(FSMItem::State curState = FSMItem::State::GETUP)
        : _curState(curState),
          _enterTime(std::chrono::steady_clock::now()),
          _lastState(curState) {
        initFSMTable();
    }

    void transferState(FSMItem::State nextState) {
        _lastState = _curState;
        _curState = nextState;
        _enterTime = std::chrono::steady_clock::now();

        cout << "[STATE] -> " << stateName(_curState) << endl;
    }

    // 新增：超时检测（主循环里周期调用）
    void tick() {
        auto to = timeoutOf(_curState);
        if (to.count() <= 0) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _enterTime);

        if (elapsed > to) {
            cout << "[TICK] " << stateName(_curState)
                 << " elapsed=" << elapsed.count() << "ms"
                 << " > timeout=" << to.count() << "ms" << endl;

            handleEvent(FSMItem::Events::EVENT_TIMEOUT);
        }
    }

    void handleEvent(FSMItem::Events event) {
        FSMItem::State curState = _curState;
        void(*action)() = nullptr;
        FSMItem::State nextState = curState;
        bool found = false;

        for (int i = 0; i < (int)_fsmTable.size(); i++) {
            if (event == _fsmTable[i]->_event && curState == _fsmTable[i]->_curState) {
                found = true;
                action = _fsmTable[i]->_action;
                nextState = _fsmTable[i]->_nextState;
                break;
            }
        }

        if (found) {
            if (action) action();
            transferState(nextState);
        } else {
            // 新增：非法事件 -> ERROR
            cout << "[ERROR] no transition: state=" << stateName(_curState)
                 << " event=" << static_cast<int>(event) << endl;

            FSMItem::on_error();
            transferState(FSMItem::State::ERROR);
        }
    }

    ~FSM() {
        for (auto p : _fsmTable) delete p;
        _fsmTable.clear();
    }
};

//测试事件变换：按照一定顺序循环变化
void testEvent(FSMItem::Events& event) {
    switch (event) {
    case FSMItem::Events::EVENT1: event = FSMItem::Events::EVENT2; break;
    case FSMItem::Events::EVENT2: event = FSMItem::Events::EVENT3; break;
    case FSMItem::Events::EVENT3: event = FSMItem::Events::EVENT1; break;
    default: break;
    }
}

int main() {
    FSM* fsm = new FSM();
    auto event = FSMItem::Events::EVENT1;

    int i = 0;
    while (i < 12) {
        // 模拟：某些轮次故意“卡住”一会儿，触发超时
        if (i == 4 || i == 9) {
            cout << "\n--- simulate blocking ... (sleep 1600ms) ---\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1600));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        // 先跑一次 tick，看看当前状态是否超时
        fsm->tick();

        cout << "event " << static_cast<int>(event) << " is coming......" << endl;
        fsm->handleEvent(event);
        cout << "fsm current state is " << static_cast<int>(fsm->_curState) << "\n" << endl;

        testEvent(event);
        i++;
    }

    delete fsm;
    return 0;
}
