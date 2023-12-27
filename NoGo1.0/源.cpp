#pragma GCC optimize(3)
#include<stdio.h>
#include<conio.h>
#include<graphics.h>
#include <climits>
#include <cstring>
#include <ctime>
#include <math.h>
#include <random>
#include <string>
#include <iostream>
#include<fstream>
#define MAXBranchNum 81
using namespace std;
//#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")
char music[11][2][30] = { {{"play res\\music0.mp3"}, {"close res\\music0.mp3"} },
{ {"play res\\music1.mp3"}, {"close res\\music1.mp3"} }, 
{ {"play res\\music2.mp3"}, {"close res\\music2.mp3"} },
    { {"play res\\music3.mp3"}, {"close res\\music3.mp3"} },
{ {"play res\\music4.mp3"}, {"close res\\music4.mp3"} },
{ {"play res\\music5.mp3"}, {"close res\\music5.mp3"} },
    { {"play res\\music6.mp3"}, {"close res\\music6.mp3"} },
    { {"play res\\music7.mp3"}, {"close res\\music7.mp3"} },
    { {"play res\\music8.mp3"}, {"close res\\music8.mp3"} },
    { {"play res\\music9.mp3"}, {"close res\\music9.mp3"} },{{},{}} };
int musicline = 0;
int board[9][9] = { 0 };
int dx[4] = { -1, 0, 1, 0 }; //行位移
int dy[4] = { 0, -1, 0, 1 }; //列位移
bool visited_by_air_judge[9][9] = { false }; //在air_judge函数判断某一点有无气时作标记，防止重复而死循环
IMAGE blackChess;
IMAGE whiteChess;
IMAGE cover;
IMAGE abyssmage;
IMAGE abyssmagecover;
IMAGE fail,failCover;
IMAGE Opponentwin;
IMAGE chessBoard;
//IMAGE chessBoardBroken;
int timeout = (int)(0.98 * (double)CLOCKS_PER_SEC);
int turnID = 0;
int helpBoard[9][9] = { 0 };                     //棋盘，黑子1，白子-1，没有棋子是0
const int chessSize = 50;
int mouseX, mouseY;
int pointX = -1, pointY = -1;
int AIX, AIY;
int myColor = 0;
int Rom;
int youLose;
struct chess{
    int X;
    int Y;
    int Color;
};
chess LastChess,LastChess1,LastChess2,nowChess;
struct point{
    int xx;
    int yy;
};
bool reStart = false;
bool chance = false;

//判断是否在棋盘内
bool inBoard_judge(int x, int y){
    return 0 <= x && x < 9 && 0 <= y && y < 9;
}

bool air_judge(int board[9][9], int x, int y){
    visited_by_air_judge[x][y] = true; //标记，表示这个位置已经搜过有无气了
    bool flag = false;
    for (int dir = 0; dir < 4; ++dir){
        int x_dx = x + dx[dir], y_dy = y + dy[dir];
        if (inBoard_judge(x_dx, y_dy)){ //界内
            if (board[x_dx][y_dy] == 0) //旁边这个位置没有棋子
                flag = true;
            if (board[x_dx][y_dy] == board[x][y] && !visited_by_air_judge[x_dx][y_dy]) //旁边这个位置是没被搜索过的同色棋
                if (air_judge(board, x_dx, y_dy))
                    flag = true;
        }
    }
    return flag;
}

bool put_available(int board[9][9], int x, int y, int color){
    if (!inBoard_judge(x, y))
        return false;
    if (board[x][y]) //如果这个点本来就有棋子
        return false;

    board[x][y] = color;
    memset(visited_by_air_judge, 0, sizeof(visited_by_air_judge)); //重置

    if (!air_judge(board, x, y)){ //如果下完这步这个点没气了,说明是自杀步，不能下
        board[x][y] = 0;
        return false;
    }

    for (int i = 0; i < 4; ++i){ //判断下完这步周围位置的棋子是否有气
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)){ //在棋盘内
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) //对于有棋子的位置（标记访问过避免死循环）
                if (!air_judge(board, x_dx, y_dy)){                      //如果导致(x_dx,y_dy)没气了，则不能下
                    board[x][y] = 0; //回溯
                    return false;
                }
        }
    }
    board[x][y] = 0; //回溯
    return true;
}

int getValidPositions(int board[9][9], int result[9][9]){
    memset(result, 0, MAXBranchNum * 4);
    int right = 0;
    for (int x = 0; x < 9; ++x){
        for (int y = 0; y < 9; ++y){
            if (put_available(board, x, y, 1)){
                right++;
                result[x][y] = 1;
            }
        }
    }
    return right;
}

class treeNode{
public:
    treeNode* parent;                 //父节点
    treeNode* children[MAXBranchNum]; //子节点
    int board[9][9];
    int childrenAction[MAXBranchNum][2];
    int childrenCount;
    int childrenCountMax;
    double value;      //该节点的总value
    int n;             //当前节点探索次数，UCB中的ni
    double UCB;        //当前节点的UCB值
    int* countPointer; //总节点数的指针
    //构造函数
    treeNode(int parentBoard[9][9], int opp_action[2], treeNode* parentPointer, int* countp){ //构造函数 treeNode *p是父类指针, int *countp应该是总探索次数的指针
        for (int i = 0; i < 9; ++i){ //把棋盘反过来，要落子方是1 ，对手是-1
            for (int j = 0; j < 9; ++j){
                board[i][j] = -parentBoard[i][j];
            }
        }
        if (opp_action[0] >= 0 && opp_action[0] < 9 && opp_action[1] >= 0 && opp_action[1] < 9)
            board[opp_action[0]][opp_action[1]] = -1;
        parent = parentPointer;
        value = 0;
        n = 0;
        childrenCount = 0;     //已经拓展的子节点数
        countPointer = countp; //count的指针
        evaluate();            //计算能下的位置,修改了childrenCountMax、childrenAction
    }
    treeNode* treeRules(){ //搜索法则
        //如果没有位置下了（终局）
        if (childrenCountMax == 0){
            return this; //到达终局当前叶节点
        }

        //如果是叶节点，Node Expansion，拓展下一层节点
        if (childrenCountMax > childrenCount){
            treeNode* newNode = new treeNode(board, childrenAction[childrenCount], this, countPointer); //拓展一个子节点
            children[childrenCount] = newNode;
            childrenCount++; //已拓展的子节点数++
            return newNode;
        }

        //计算当前节点的每个子节点的UCB值（点亮某个节点）
        for (int i = 0; i < childrenCount; ++i){
            children[i]->UCB = children[i]->value / double(children[i]->n) + 0.272 * sqrt(log(double(*countPointer)) / double(children[i]->n)); //UCB公式
        }
        int bestChild = 0;
        double maxUCB = 0;

        //找出所有子节点中UCB值最大的子节点
        for (int i = 0; i < childrenCount; ++i){
            if (maxUCB < children[i]->UCB){
                bestChild = i;
                maxUCB = children[i]->UCB;
            }
        }
        return children[bestChild]->treeRules(); //对UCB最大的子节点进行下一层搜索
    }
    //模拟
    double simulation(){
        int simulationBoard[9][9];
        int board_opp[9][9]; //对手棋盘
        int res[9][9];
        double x = 0, y = 0;
        vector<int> available_list; //合法位置表
        vector<int> op_available_list;
        for (int p = 0; p < 10; p++){
            for (int i = 0; i < 9; ++i){
                for (int j = 0; j < 9; ++j){
                    simulationBoard[i][j] = board[i][j];
                }
            }
            for (int q = 0; q < 10; q++){
                int flag = 1;
                for (int i = 0; i < 9; i++)
                    for (int j = 0; j < 9; j++)
                        if (put_available(simulationBoard, i, j, 1)){
                            available_list.push_back(i * 9 + j);
                            flag = 0;
                        }
                if (flag)
                    break;
                flag = 1;
                int result = available_list[rand() % available_list.size()];
                simulationBoard[result / 9][result % 9] = 1;
                for (int i = 0; i < 9; i++)
                    for (int j = 0; j < 9; j++)
                        if (put_available(simulationBoard, i, j, -1)){
                            op_available_list.push_back(i * 9 + j);
                            flag = 0;
                        }
                if (flag)
                    break;
                result = op_available_list[rand() % op_available_list.size()];
                simulationBoard[result / 9][result % 9] = -1;
            }
            for (int i = 0; i < 9; ++i){
                for (int j = 0; j < 9; ++j){
                    board_opp[i][j] = -simulationBoard[i][j];
                }
            }
            x += getValidPositions(simulationBoard, res);     //落子方可下位置数
            y += getValidPositions(board_opp, res); //非落子方可下位置数
        }
            return (double)(x - y)/10;
    }
    void backup(double deltaValue){ //回传估值,从当前叶节点以及往上的每一个父节点都加上估值
        treeNode* node = this;
        int side = 0;
        while (node != nullptr){ //当node不是根节点的父节点时
            if (side == 1){ //落子方
                node->value += deltaValue;
                side--;
            }
            else{ //非落子方
                node->value -= deltaValue;
                side++;
            }
            node->n++; //当前节点被探索次数++
            node = node->parent;
        }
    }

private:
    void evaluate(){ //计算能下的位置,修改了childrenCountMax、childrenAction
        int result[9][9];
        int validPositionCount = getValidPositions(board, result); //能下的位置数
        int validPositions[MAXBranchNum];                          //能下的位置坐标
        int availableNum = 0;
        for (int i = 0; i < 9; ++i){
            for (int j = 0; j < 9; ++j){
                if (result[i][j]){
                    validPositions[availableNum] = i * 9 + j; //可下的位置
                    availableNum++;                           //可下的位置数
                }
            }
        }
        childrenCountMax = validPositionCount; //总共能下的位置数
        for (int i = 0; i < validPositionCount; ++i){
            childrenAction[i][0] = validPositions[i] / 9;
            childrenAction[i][1] = validPositions[i] % 9;
        }
    }
};

void init(){
    initgraph(1200, 800);
    loadimage(&chessBoard, "res\\board.bmp",800,800,true);
    //loadimage(&chessBoardBroken, "res\\chessboardbroken.jpg", 800, 800, true);
    loadimage(&blackChess, "res\\blackChess.bmp", chessSize, chessSize, true);
    loadimage(&whiteChess, "res\\whiteChess.bmp", chessSize, chessSize, true);
    loadimage(&cover, "res\\cover.bmp", chessSize, chessSize, true);
    loadimage(&abyssmage, "res\\Opponent.png", 400, 500, true);
    //loadimage(&abyssmagecover, "res\\深渊法师cover.jpg", 400, 400, true);
    loadimage(&fail, "res\\fail.jpg", 300, 100, true);
    loadimage(&failCover, "res\\cailcover.jpg", 300, 100, true);
    loadimage(&Opponentwin, "res\\Opponent2.png", 400, 500, true);
    putimage(0, 0, &chessBoard);
    //putimage(800, 0, &abyssmagecover, SRCAND);
    putimage(800, 0, &abyssmage, SRCPAINT);
    setfillcolor(RGB(225, 133, 72));
    fillrectangle(850, 575, 990, 625);
    fillrectangle(1010, 575, 1150, 625);
    fillrectangle(850, 650, 990, 700);
    fillrectangle(1010, 650, 1150, 700);
    fillrectangle(850, 725, 1150, 775);
    setbkmode(TRANSPARENT);
    settextstyle(40, 0, "楷体");
    LastChess.X = -1;
    LastChess.Y = -1;
    LastChess.Color = 1;
    LastChess1.X = -1;
    LastChess1.Y = -1;
    LastChess1.Color = -1;
    LastChess2.X = -1;
    LastChess2.Y = -1;
    LastChess2.Color = 1;
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            board[i][j] = 0;
    //turnID = 0;
}

point closest(int a, int b){
    point result;
    pointX = (a / 80) - 1;
    pointY = (b / 80) - 1;
    int flag = 0;//防止四点都不行
    int dX[4] = { 0,1,0,1 }, dY[4] = { 0,0,1,1 };
    for (int i = 0; i < 4; i++){
        if (inBoard_judge(pointX + dX[i], pointY + dY[i]) && !board[pointX + dX[i]][pointY + dY[i]])
            if (((pointX + dX[i] + 1) * 80 - a) * ((pointX + dX[i] + 1) * 80 - a) < 1600 && ((pointY + dY[i] + 1) * 80 - b) * ((pointY + dY[i] + 1) * 80 - b) < 1600){
                pointX += dX[i];
                pointY += dY[i];
                flag = 1;
            }
    }
    if (flag){
        result.xx = pointX;
        result.yy = pointY;
    }
    if (!flag){
        result.xx = -1;
        result.yy = -1;
    }
    return result;
}

void chessDown(int x, int y, int color){
    if (LastChess.X != -1){
        if (LastChess.Color == 1){
            putimage((LastChess.X + 1) * 80 - chessSize / 2, (LastChess.Y + 1) * 80 - chessSize / 2, &cover, SRCAND);
            putimage((LastChess.X + 1) * 80 - chessSize / 2, (LastChess.Y + 1) * 80 - chessSize / 2, &blackChess, SRCPAINT);
        }
        else if (LastChess.Color == -1){
            putimage((LastChess.X + 1) * 80 - chessSize / 2, (LastChess.Y + 1) * 80 - chessSize / 2, &cover, SRCAND);
            putimage((LastChess.X + 1) * 80 - chessSize / 2, (LastChess.Y + 1) * 80 - chessSize / 2, &whiteChess, SRCPAINT);
        }
    }
    LastChess2.X = LastChess1.X;
    LastChess2.Y = LastChess1.Y;
    LastChess2.Color = LastChess1.Color;
    LastChess1.X = LastChess.X;
    LastChess1.Y = LastChess.Y;
    LastChess1.Color = LastChess.Color;
    LastChess.X = x;
    LastChess.Y = y;
    LastChess.Color = color;
    int X = (x + 1) * 80 - chessSize / 2;
    int Y = (y + 1) * 80 - chessSize / 2;
    if (color != -1){
        putimage(X, Y, &cover, SRCAND);
        putimage(X, Y, &blackChess, SRCPAINT);
        POINT pts[] = { {(x + 1) * 80,(y + 1) * 80}, {(x + 1) * 80+15,(y + 1) * 80}, {(x + 1) * 80,(y + 1) * 80+15} };
        setfillcolor(RGB(255, 255, 255)); 
        solidpolygon(pts, 3);
    }
    else if (color == -1){
        putimage(X, Y, &cover, SRCAND);
        putimage(X, Y, &whiteChess, SRCPAINT);
        POINT pts[] = { {(x + 1) * 80,(y + 1) * 80}, {(x + 1) * 80 + 15,(y + 1) * 80}, {(x + 1) * 80,(y + 1) * 80 + 15} };
        setfillcolor(RGB(0, 0, 0));
        solidpolygon(pts, 3);
        board[x][y] = -1;
    }
    if (myColor == color)
        board[x][y] = 1;
    else
        board[x][y] = -1;
    turnID++;
    //start = clock();
}

bool checkOver(int x, int y){
    memset(visited_by_air_judge, 0, sizeof(visited_by_air_judge));
    if (!air_judge(board, x, y))
        return true;
    for (int i = 0; i < 4; i++){ //判断下完这步周围位置的棋子是否有气
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)){ //在棋盘内
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) //对于有棋子的位置（标记访问过避免死循环）
                if (!air_judge(board, x_dx, y_dy)) {                            //如果导致(x_dx,y_dy)没气了，则不能下
                    return true;
                }
        }
    }
    return false;
}

void manGo(){
    PlaySound("res\\click.wav", NULL, SND_FILENAME | SND_ASYNC);
    chessDown(closest(mouseX, mouseY).xx, closest(mouseX, mouseY).yy, myColor);
    chance = true;
}

void AIGo(){
    bool visited_by_air_judge[9][9] = { false };
    int count = 0; //总计算的节点数（总探索次数，UCB中的N）
    srand(clock());
    int start = clock();
    int timeout = (int)(0.98 * (double)CLOCKS_PER_SEC);
    int opp_action[2] = { LastChess.X, LastChess.Y }; //对面上一步走了哪里
    treeNode rootNode(board, opp_action, nullptr, &count); //创建根节点，根节点的父节点为空
    while (clock() - start < timeout){
        count++;                                //计算的节点数++
        treeNode* node = rootNode.treeRules(); //拓展一次，node指向的是一次拓展的叶节点
        double result = node->simulation();     //结果估值
        node->backup(result);
    }
    int bestChildren[MAXBranchNum] = { 0 }; //所有最优子节点的序号
    int bestChildrenNum = 0;              //最优子节点个数
    int maxValue = INT_MIN;  //当前最优子节点分数
    for (int i = 0; i < rootNode.childrenCount; ++i){
        if (maxValue < rootNode.children[i]->value){
            //重置
            memset(bestChildren, 0, sizeof(bestChildren));
            bestChildrenNum = 0;

            bestChildren[bestChildrenNum++] = i;
            maxValue = rootNode.children[i]->value;
        }
        else if (maxValue == rootNode.children[i]->value){
            bestChildren[bestChildrenNum++] = i;
        }
    }
    int random = rand() % bestChildrenNum;                         //在所有最优中任选一个
    int* bestAction = rootNode.childrenAction[bestChildren[random]]; //最优子节点对应走法
    AIX = bestAction[0];
    AIY = bestAction[1];
}

void AIHelp(){
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            board[i][j] = -board[i][j];
    //检查是否终局
    youLose = 1;
    for (int i = 0; i < 9; i++){
        for (int j = 0; j < 9; j++)
            if (put_available(board, i, j, -1)){
                youLose = 0;
                break;
            }
        if (!youLose)
            break;
    }
    if(!youLose)
        AIGo();
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            board[i][j] = -board[i][j];
}

void save(){ 
    mciSendString(_T(music[musicline][1]), NULL, 0, NULL);
    ofstream outfile("data.dat");
    outfile << LastChess.X << ' ';
    outfile << LastChess.Y << ' ';
    outfile << LastChess.Color << ' ';
    outfile << myColor << ' ';
    outfile << musicline << ' ';
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            outfile << board[i][j] << ' ';
    outfile.close();
}

void systemWrong(){
    setfillcolor(RGB(0, 0, 0));
    fillrectangle(200, 300, 600, 400);
    settextcolor(RGB(255, 255, 255));
    settextstyle(80, 0, "楷体");
    outtextxy(210, 310, "系统错误！");
    Sleep(700);
    putimage(0, 0, &chessBoard);
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (((i + j) % 2 == 0)&&board[i][j])
                chessDown(i, j, clock() % 2 * 2 - 1);
    Sleep(500);
    putimage(0, 0, &chessBoard);
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (((i + j) % 2 == 1) && board[i][j])
                chessDown(i, j, clock() % 2 * 2 - 1);
    Sleep(300);
    putimage(0, 0, &chessBoard);
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (clock() % 4 == 0)
                chessDown(i, j, clock() % 2 * 2 - 1);
    Sleep(200);
    mciSendString("play res\\AIlose.mp3", NULL, 0, NULL);
    Sleep(500);
    closegraph();
}

int startGame(){
    void title();
    init();
    double Start = 0,end;
    bool slow = false;
    chance = false;
    settextcolor(RGB(0, 0, 0));
    outtextxy(860, 730, "保存并退出");
    settextcolor(RGB(0, 0, 0));
    outtextxy(860, 655, "认输");
    settextcolor(RGB(0, 0, 0));
    outtextxy(1020, 655, "音乐");
    settextcolor(RGB(0, 0, 0));
    outtextxy(860, 580, "提示");
    settextcolor(RGB(0, 0, 0));
    outtextxy(1020, 580, "悔棋");
    if (myColor==-1&&!reStart){
        mciSendString(music[musicline][0], NULL, 0, NULL);
        AIGo();
        chessDown(AIX, AIY, -myColor);
    }
    else if (reStart){
        ifstream infile("data.dat");
        infile >> LastChess.X;
        infile >> LastChess.Y;
        infile >> LastChess.Color;
        infile >> myColor;
        infile >> musicline;
        mciSendString(music[musicline][0], NULL, 0, NULL);
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++){
                infile >> board[i][j];
                if (board[i][j] == 1) {
                    chessDown(i, j, myColor);
                }
                if (board[i][j] == -1) {
                    chessDown(i, j, -myColor);
                }
            }
        infile.close();
        reStart = false;
    }
    else  mciSendString(music[musicline][0], NULL, 0, NULL);
    while (1){
        ExMessage msg = getmessage(EX_MOUSE);
        mouseX = msg.x;
        mouseY = msg.y;
        //save and quit
        if (mouseX >= 850 && mouseX <= 1150 && mouseY >= 725 && mouseY <= 775){
            settextcolor(RGB(255, 0, 0));
            outtextxy(860, 730, "保存并退出");   //输出文字
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(860, 730, "保存并退出");
        }
        //renshu
        if (mouseX >= 850 && mouseX <= 990 && mouseY >= 650 && mouseY <= 700){
            settextcolor(RGB(255, 0, 0));
            outtextxy(860, 655, "认输");   //输出文字
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(860, 655, "认输");
        }
        //bgm
        if (mouseX >= 1020 && mouseX <= 1150 && mouseY >= 650 && mouseY <= 700){
            settextcolor(RGB(255, 0, 0));
            outtextxy(1020, 655, "音乐");   //输出文字
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(1020, 655, "音乐");
        }
        //提示
        if (mouseX >= 850 && mouseX <= 990 && mouseY >= 575 && mouseY <= 625){
            settextcolor(RGB(255, 0, 0));
            outtextxy(860, 580, "提示");   //输出文字
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(860, 580, "提示");
        }
        //悔棋
        if (mouseX >= 1010 && mouseX <= 1150 && mouseY >= 575 && mouseY <= 625){
            settextcolor(RGB(255, 0, 0));
            outtextxy(1020, 580, "悔棋");   //输出文字
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(1020, 580, "悔棋");
        }
        if (msg.message == WM_LBUTTONDOWN) {
            //save and back to title
            if (mouseX >= 850 && mouseX <= 1150 && mouseY >= 725 && mouseY <= 775) {
                PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                mciSendString(_T(music[musicline][1]), NULL, 0, NULL);
                save();
                Sleep(500);
                title();
            }
            //lose
            if (mouseX >= 850 && mouseX <= 990 && mouseY >= 650 && mouseY <= 700) {
                PlaySound("res\\click2.wav", NULL, SND_FILENAME);
                mciSendString(_T(music[musicline][1]), NULL, 0, NULL);
                PlaySound("res\\fail.wav", NULL, SND_FILENAME | SND_ASYNC);
                putimage(800, 0, &Opponentwin, SRCCOPY);
                putimage(850, 450, &failCover, SRCAND);
                putimage(850, 450, &fail, SRCPAINT);
                setfillcolor(RGB(225, 133, 72));
                fillrectangle(850, 650, 990, 700);
                fillrectangle(850, 725, 1150, 775);
                while (1) {
                    ExMessage msg = getmessage(EX_MOUSE);
                    mouseX = msg.x;
                    mouseY = msg.y;
                    //try again
                    if (mouseX >= 850 && mouseX <= 990 && mouseY >= 650 && mouseY <= 700) {
                        settextcolor(RGB(255, 0, 0));
                        outtextxy(860, 655, "新对局");   //输出文字
                    }
                    else {
                        settextcolor(RGB(0, 0, 0));
                        outtextxy(860, 655, "新对局");
                    }
                    //quit
                    if (mouseX >= 850 && mouseX <= 1150 && mouseY >= 725 && mouseY <= 775) {
                        settextcolor(RGB(255, 0, 0));
                        outtextxy(860, 730, "退出");   //输出文字
                    }
                    else {
                        settextcolor(RGB(0, 0, 0));
                        outtextxy(860, 730, "退出");
                    }
                    if (msg.message == WM_LBUTTONDOWN) {
                        //try again
                        if (mouseX >= 850 && mouseX <= 1150 && mouseY >= 650 && mouseY <= 700) {
                            PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                            mciSendString(music[musicline][0], NULL, 0, NULL);
                            startGame();
                        }
                        //quit
                        if (mouseX >= 850 && mouseX <= 1150 && mouseY >= 725 && mouseY <= 775) {
                            PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                            mciSendString(_T(music[musicline][1]), NULL, 0, NULL);
                            title();
                        }
                    }
                }
            }
            //音乐
            if (mouseX >= 1020 && mouseX <= 1150 && mouseY >= 650 && mouseY <= 700) {
                mciSendString(_T(music[musicline][1]), NULL, 0, NULL);
                musicline++;
                if (musicline == 11)musicline = 0;
                mciSendString(music[musicline][0], NULL, 0, NULL);
            }
            //提示
            if (mouseX >= 850 && mouseX <= 990 && mouseY >= 575 && mouseY <= 625) {
                PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                AIHelp();
                setfillcolor(RGB(225, 133, 72));
                fillrectangle(850, 575, 990, 625);
                if (!youLose) {
                    outtextxy(860, 580, '(');
                    outtextxy(880, 580, '0' + AIX);
                    outtextxy(900, 580, ',');
                    outtextxy(920, 580, '0' + AIY);
                    outtextxy(940, 580, ')');
                }
                else
                    outtextxy(860, 580, "投了吧");
                Sleep(2000);
                setfillcolor(RGB(225, 133, 72));
                fillrectangle(850, 575, 990, 625);
            }
            //悔棋
            if (mouseX >= 1010 && mouseX <= 1150 && mouseY >= 575 && mouseY <= 625 && chance) {
                PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                chance = false;
                Rom = clock() % 9;
                board[LastChess.X][LastChess.Y] = 0;
                board[LastChess1.X][LastChess1.Y] = 0;
                LastChess.X = LastChess2.X;
                LastChess.Y = LastChess2.Y;
                LastChess.Color = LastChess2.Color;
                LastChess1.X = -1;
                LastChess1.Y = -1;
                LastChess1.Color = 1;
                LastChess2.X = -1;
                LastChess2.Y = -1;
                LastChess2.Color = 1;
                putimage(0, 0, &chessBoard);
                for (int i = 0; i < 9; i++)
                    for (int j = 0; j < 9; j++) {
                        if (board[i][j] == 1) {
                            putimage((i + 1) * 80 - chessSize / 2, (j + 1) * 80 - chessSize / 2, &cover, SRCAND);
                            putimage((i + 1) * 80 - chessSize / 2, (j + 1) * 80 - chessSize / 2, &blackChess, SRCPAINT);
                        }
                        if (board[i][j] == -1) {
                            putimage((i + 1) * 80 - chessSize / 2, (j + 1) * 80 - chessSize / 2, &cover, SRCAND);
                            putimage((i + 1) * 80 - chessSize / 2, (j + 1) * 80 - chessSize / 2, &whiteChess, SRCPAINT);
                        }
                    }
                if (LastChess.Color == 1) {
                    POINT pts[] = { {(LastChess.X + 1) * 80,(LastChess.Y + 1) * 80}, {(LastChess.X + 1) * 80 + 15,(LastChess.Y + 1) * 80}, {(LastChess.X + 1) * 80,(LastChess.Y + 1) * 80 + 15} };
                    setfillcolor(RGB(255, 255, 255));
                    solidpolygon(pts, 3);
                }
                if (LastChess.Color == -1) {
                    POINT pts[] = { {(LastChess.X + 1) * 80,(LastChess.Y + 1) * 80}, {(LastChess.X + 1) * 80 + 15,(LastChess.Y + 1) * 80}, {(LastChess.X + 1) * 80,(LastChess.Y + 1) * 80 + 15} };
                    setfillcolor(RGB(0, 0, 0));
                    solidpolygon(pts, 3);
                }
                //复原两回合前棋局
            }
            //下棋
            if (msg.x <= 760 && msg.x >= 40 && msg.y >= 40 && msg.y <= 760 && inBoard_judge(closest(msg.x, msg.y).xx, closest(msg.x, msg.y).yy) && !board[closest(msg.x, msg.y).xx][closest(msg.x, msg.y).yy]){
                bool firstTurn = true;
                for (int i = 0; i < 9; i++){
                    for (int j = 0; j < 9; j++)
                        if (board[i][j]){
                            firstTurn = false;
                            break;
                        }
                    if (!firstTurn)
                        break;
                }
                manGo();               
                //若输棋
                if (checkOver(pointX, pointY)||(firstTurn&&pointX==4&&pointY==4)){
                    mciSendString(_T(music[musicline][1]), NULL, 0, NULL);
                    PlaySound("res\\fail.wav", NULL, SND_FILENAME | SND_ASYNC);
                    putimage(800, 0, &Opponentwin, SRCCOPY);
                    putimage(850, 450, &failCover, SRCAND);
                    putimage(850, 450, &fail, SRCPAINT);
                    setfillcolor(RGB(225, 133, 72));
                    fillrectangle(850, 650, 990, 700);
                    fillrectangle(850, 725, 1150, 775);
                    while (1) {
                        ExMessage msg = getmessage(EX_MOUSE);
                        mouseX = msg.x;
                        mouseY = msg.y;

                        //悔棋
                        if (mouseX >= 1010 && mouseX <= 1150 && mouseY >= 575 && mouseY <= 625) {
                            settextcolor(RGB(255, 0, 0));
                            outtextxy(1020, 580, "悔棋");   //输出文字
                        }
                        else {
                            settextcolor(RGB(0, 0, 0));
                            outtextxy(1020, 580, "悔棋");
                        }

                        //再凹一次
                        if (mouseX >= 850 && mouseX <= 990 && mouseY >= 650 && mouseY <= 700) {
                            settextcolor(RGB(255, 0, 0));
                            outtextxy(860, 655, "新对局");   //输出文字
                        }
                        else {
                            settextcolor(RGB(0, 0, 0));
                            outtextxy(860, 655, "新对局");
                        }

                        //quit
                        if (mouseX >= 850 && mouseX <= 1020 && mouseY >= 725 && mouseY <= 775){
                            settextcolor(RGB(255, 0, 0)); 
                            outtextxy(860, 730, "退出");   //输出文字
                        }
                        else{
                            settextcolor(RGB(0, 0, 0));
                            outtextxy(860, 730, "退出");
                        }
                        if (msg.message == WM_LBUTTONDOWN){
                            //悔棋
                            if (mouseX >= 1010 && mouseX <= 1150 && mouseY >= 575 && mouseY <= 625 && chance){
                                PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                                chance = false;
                                board[LastChess.X][LastChess.Y] = 0;
                                LastChess.X = LastChess1.X;
                                LastChess.Y = LastChess1.Y;
                                LastChess.Color = LastChess1.Color; 
                                reStart = true;
                                save();
                                startGame();
                            }
                            //try again
                            if (mouseX >= 850 && mouseX <= 1020 && mouseY >= 650 && mouseY <= 700){
                                //mciSendString("stop res\\bgm.mp3", NULL, 0, NULL);
                                PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                                mciSendString(music[musicline][0], NULL, 0, NULL);
                                startGame();
                            }
                            //quit
                            if (mouseX >= 850 && mouseX <= 1150 && mouseY >= 725 && mouseY <= 775){
                                PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                                mciSendString(_T(music[musicline][1]), NULL, 0, NULL);
                                Sleep(500);
                                title();
                            }
                        }
                    }
                }
                int AILose = 1;
                //检查是否还有能下的地方
                for (int i = 0; i < 9; i++){
                    for (int j = 0; j < 9; j++)
                        if (put_available(board, i, j, -1)){
                            AILose = 0;
                            break;
                        }
                    if (!AILose)
                        break;
                }
                //有则继续下
                if (!AILose){
                    AIGo();
                    chessDown(AIX, AIY, -myColor);
                    firstTurn = false;
                }
                //无则玩家获胜
                if (AILose)
                    systemWrong();
            }
        }
    }
    closegraph();
    return 0;
}

void title(){
    mciSendString("pause res\\title.mp3", NULL, 0, NULL);
    mciSendString("resume res\\title.mp3", NULL, 0, NULL);
    initgraph(1200, 800);
    loadimage(NULL, "res\\title.bmp", 1200, 800, true);
    setfillcolor(RGB(225, 133, 72));
    fillrectangle(100, 700, 300, 750);
    fillrectangle(350, 700, 550, 750);
    fillrectangle(600, 700, 800, 750);
    setbkmode(TRANSPARENT);
    settextstyle(45, 0, "楷体");    //设置字体的风格
    while (1){
        ExMessage msg = getmessage(EX_MOUSE);
        mouseX = msg.x;
        mouseY = msg.y;
        //quit
        if (mouseX >= 600 && mouseX <= 800 && mouseY >= 700 && mouseY <= 750){
            settextcolor(RGB(255, 0, 0));
            outtextxy(650,705 , "退出");   //输出文字
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(650, 705, "退出");
        }
        //continue
        if (mouseX >= 350 && mouseX <= 550 && mouseY >= 700 && mouseY <= 750){
            settextcolor(RGB(255, 0, 0));
            outtextxy(360, 705, "继续游戏");   //输出文字
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(360, 705, "继续游戏");
        }
        //New Game
        if (mouseX >= 100 && mouseX <= 300 && mouseY >= 700 && mouseY <= 750){
            settextcolor(RGB(255, 0, 0));
            outtextxy(130, 705, "新游戏");   //输出文字
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(130, 705, "新游戏");
        }
        //点击事件
        if (msg.message == WM_LBUTTONDOWN){
            //New Game
            if (mouseX >= 100 && mouseX <= 300 && mouseY >= 700 && mouseY <= 750){
                PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                setfillcolor(RGB(225, 133, 72));
                fillrectangle(200, 700, 300, 750);
                setfillcolor(RGB(225, 133, 72));
                fillrectangle(100, 700, 200, 750);
                //fillrectangle(1060, 350, 1150, 400);
                while (1){
                    ExMessage msg = getmessage(EX_MOUSE);
                    mouseX = msg.x;
                    mouseY = msg.y;
                    //执黑
                    if (mouseX >= 100 && mouseX <= 190 && mouseY >= 700 && mouseY <= 750){
                        settextcolor(RGB(255, 0, 0));
                        outtextxy(100, 705, "执黑");   //输出文字
                    }
                    else{
                        settextcolor(RGB(0, 0, 0));
                        outtextxy(100, 705, "执黑");
                    }
                    //执白
                    if (mouseX >= 210 && mouseX <= 300 && mouseY >= 700 && mouseY <= 750){
                        settextcolor(RGB(255, 0, 0));
                        outtextxy(210, 705, "执白");   //输出文字
                    }
                    else{
                        settextcolor(RGB(0, 0, 0));
                        outtextxy(210, 705, "执白");
                    }
                    if (msg.message == WM_LBUTTONDOWN){
                        if (mouseX >= 100 && mouseX <= 190 && mouseY >= 700 && mouseY <= 750){//执黑
                            PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                            myColor = 1;
                            Rom = clock() % 4;
                            mciSendString("pause res\\title.mp3", NULL, 0, NULL);
                            mciSendString(music[musicline][0], NULL, 0, NULL);
                            startGame();
                        }
                        if (mouseX >= 210 && mouseX <= 300 && mouseY >= 700 && mouseY <= 750){
                            PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                            myColor = -1;
                            Rom = clock() % 4;
                            mciSendString(_T("pause res\\title.mp3"), NULL, 0, NULL);
                            mciSendString(music[musicline][0], NULL, 0, NULL);
                            startGame();
                        }
                    }
                }               
            }
            //comtinue
            if (mouseX >= 350 && mouseX <= 550 && mouseY >= 700 && mouseY <= 750){
                mciSendString("stop res\\title.mp3", NULL, 0, NULL);
                PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                reStart = true;
                startGame();
            }
            //quit
            if (mouseX >= 600 && mouseX <= 800 && mouseY >= 700 && mouseY <= 750){
                mciSendString(_T("close res\\title.mp3"), NULL, 0, NULL);
                mciSendString(_T(music[musicline][1]), NULL, 0, NULL);
                PlaySound("res\\click2.wav", NULL, SND_FILENAME | SND_ASYNC);
                closegraph();
            }
        }
    }
}

int main(){ 
    mciSendString("play res\\title.mp3", NULL, 0, NULL);
    title();
    return 0;
}