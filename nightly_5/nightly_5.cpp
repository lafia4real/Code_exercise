/*
状态机学习，包含3-5个状态
此状态机如下：
事物：学生
状态：起床、上学、吃饭、写作业、睡觉
切换状态通过执行相应的事件进行转移
*/
#include <iostream>
#include <vector>
#include <stdio.h>

using namespace std;

//状态项
class FSMItem{
    friend class FSM;           //友元类，此类说明可以访问FSMItem的所有成员(私有和保护成员都可以访问)

    private:
        static void getup(){
            cout << "you should get up!!" << endl;
        }
        static void go_school(){
            cout << "you should go school!!" << endl;
        }
        static void eat(){
            cout << "you should eat!!" << endl;
        }
        static void do_homework(){
            cout << "you should do homework!!" << endl;
        }
        static void go_sleep(){
            cout << "you should go sleep!!" << endl;
        }

    public:
        enum class State{             //枚举所有可能的状态
            GETUP = 0,
            GO_SCHOOL,
            EAT,
            DO_HOMEWORK,
            GO_SLEEP
        };
        enum class Events{            //枚举所有可能触发状态转换的事件
            EVENT1 = 0,
            EVENT2,
            EVENT3
        };
        //初始化构造函数
        FSMItem(State curState,Events event,void(*action)(),State nextState)
            :_curState(curState),_event(event),_action(action),_nextState(nextState){}

    //状态和事件
    private:
        State _curState;        //现态
        Events _event;          //条件
        void(*_action)();       //动作
        State _nextState;       //次态
};

class FSM{
    private:
        vector<FSMItem*> _fsmTable;              //定义私有变量，用来存储状态转移表
        //根据状态图初始化状态转移表
        void initFSMTable(){
            _fsmTable.push_back(new FSMItem(FSMItem::State::GETUP,FSMItem::Events::EVENT1,&FSMItem::getup,FSMItem::State::GO_SCHOOL));
            _fsmTable.push_back(new FSMItem(FSMItem::State::GO_SCHOOL,FSMItem::Events::EVENT2,&FSMItem::go_school,FSMItem::State::EAT));
            _fsmTable.push_back(new FSMItem(FSMItem::State::EAT,FSMItem::Events::EVENT3,&FSMItem::eat,FSMItem::State::DO_HOMEWORK));
            _fsmTable.push_back(new FSMItem(FSMItem::State::DO_HOMEWORK,FSMItem::Events::EVENT1,&FSMItem::do_homework,FSMItem::State::GO_SLEEP));
            _fsmTable.push_back(new FSMItem(FSMItem::State::GO_SLEEP,FSMItem::Events::EVENT2,&FSMItem::go_sleep,FSMItem::State::GETUP));
        }

    public:
        FSM(FSMItem::State curState = FSMItem::State::GETUP):_curState(curState){
            initFSMTable();
        }

        //状态转移
        void transferState(FSMItem::State nextState){
            _curState = nextState;
        }

        //当接收到一个事件时，遍历状态转移表寻找匹配当前状态以及事件的状态项
        //如果找到匹配项，则设置标志flag为真，记录对应的action函数指针和nextstate
        void handleEvent(FSMItem::Events event){
            FSMItem::State curState = _curState;        //现态
            void(*action)() = nullptr;                  //动作
            FSMItem::State nextState;                   //次态
            bool flag = false;
            for (int i = 0; i < _fsmTable.size(); i++){
                if(event == _fsmTable[i]->_event && curState == _fsmTable[i]-> _curState){
                    flag = true;
                    action = _fsmTable[i]->_action;
                    nextState = _fsmTable[i]->_nextState;
                    break;
                
                }
            }
            //找到对应的状态项，调用对应的动作函数action，然后调用transferState函数更新状态机到新状态
            if(flag){
                if(action){
                    action();
                }
                transferState(nextState);
            }
        }
    //公共部分定义一个成员变量，表示有限状态机的当前状态，可供外部访问
    public:
        FSMItem::State _curState;      

    public:
        ~FSM(){
            for(auto p : _fsmTable){
                delete p;
            }
            _fsmTable.clear();
        }
};
 
//测试事件变换，用来改变传入事件的值，使其按照一定的顺序循环变化
void testEvent(FSMItem::Events& event){
    switch (event){
    case FSMItem::Events::EVENT1:
        event = FSMItem::Events::EVENT2;   //如果事件为event1，则将事件更改为event2
        break;
    case FSMItem::Events::EVENT2:
        event = FSMItem::Events::EVENT3;
        break;
    case FSMItem::Events::EVENT3:
        event = FSMItem::Events::EVENT1;
        break;
    }
}
 
int main(){
    FSM *fsm = new FSM();   //创建一个FSM类的实例，并将其指针存储在fsm中
    auto event = FSMItem::Events::EVENT1;
    int i = 0;
    while(i < 12){
        cout << "event " << static_cast<int>(event) << " is coming......" <<endl;
        fsm->handleEvent(event);
        cout << "fsm current state is " << static_cast<int>(fsm->_curState) << endl;
        testEvent(event);
        i++;
    }
    cout << "event: " << static_cast<int>(event) <<endl;
    cout << "curState: " << static_cast<int>(fsm->_curState) <<endl;  //打印当前状态
    delete fsm;
    return 0;
}
