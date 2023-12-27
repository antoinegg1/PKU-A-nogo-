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
int dx[4] = { -1, 0, 1, 0 }; //��λ��
int dy[4] = { 0, -1, 0, 1 }; //��λ��
bool visited_by_air_judge[9][9] = { false }; //��air_judge�����ж�ĳһ��������ʱ����ǣ���ֹ�ظ�����ѭ��
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
int helpBoard[9][9] = { 0 };                     //���̣�����1������-1��û��������0
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

//�ж��Ƿ���������
bool inBoard_judge(int x, int y){
    return 0 <= x && x < 9 && 0 <= y && y < 9;
}

bool air_judge(int board[9][9], int x, int y){
    visited_by_air_judge[x][y] = true; //��ǣ���ʾ���λ���Ѿ��ѹ���������
    bool flag = false;
    for (int dir = 0; dir < 4; ++dir){
        int x_dx = x + dx[dir], y_dy = y + dy[dir];
        if (inBoard_judge(x_dx, y_dy)){ //����
            if (board[x_dx][y_dy] == 0) //�Ա����λ��û������
                flag = true;
            if (board[x_dx][y_dy] == board[x][y] && !visited_by_air_judge[x_dx][y_dy]) //�Ա����λ����û����������ͬɫ��
                if (air_judge(board, x_dx, y_dy))
                    flag = true;
        }
    }
    return flag;
}

bool put_available(int board[9][9], int x, int y, int color){
    if (!inBoard_judge(x, y))
        return false;
    if (board[x][y]) //�������㱾����������
        return false;

    board[x][y] = color;
    memset(visited_by_air_judge, 0, sizeof(visited_by_air_judge)); //����

    if (!air_judge(board, x, y)){ //��������ⲽ�����û����,˵������ɱ����������
        board[x][y] = 0;
        return false;
    }

    for (int i = 0; i < 4; ++i){ //�ж������ⲽ��Χλ�õ������Ƿ�����
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)){ //��������
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) //���������ӵ�λ�ã���Ƿ��ʹ�������ѭ����
                if (!air_judge(board, x_dx, y_dy)){                      //�������(x_dx,y_dy)û���ˣ�������
                    board[x][y] = 0; //����
                    return false;
                }
        }
    }
    board[x][y] = 0; //����
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
    treeNode* parent;                 //���ڵ�
    treeNode* children[MAXBranchNum]; //�ӽڵ�
    int board[9][9];
    int childrenAction[MAXBranchNum][2];
    int childrenCount;
    int childrenCountMax;
    double value;      //�ýڵ����value
    int n;             //��ǰ�ڵ�̽��������UCB�е�ni
    double UCB;        //��ǰ�ڵ��UCBֵ
    int* countPointer; //�ܽڵ�����ָ��
    //���캯��
    treeNode(int parentBoard[9][9], int opp_action[2], treeNode* parentPointer, int* countp){ //���캯�� treeNode *p�Ǹ���ָ��, int *countpӦ������̽��������ָ��
        for (int i = 0; i < 9; ++i){ //�����̷�������Ҫ���ӷ���1 ��������-1
            for (int j = 0; j < 9; ++j){
                board[i][j] = -parentBoard[i][j];
            }
        }
        if (opp_action[0] >= 0 && opp_action[0] < 9 && opp_action[1] >= 0 && opp_action[1] < 9)
            board[opp_action[0]][opp_action[1]] = -1;
        parent = parentPointer;
        value = 0;
        n = 0;
        childrenCount = 0;     //�Ѿ���չ���ӽڵ���
        countPointer = countp; //count��ָ��
        evaluate();            //�������µ�λ��,�޸���childrenCountMax��childrenAction
    }
    treeNode* treeRules(){ //��������
        //���û��λ�����ˣ��վ֣�
        if (childrenCountMax == 0){
            return this; //�����վֵ�ǰҶ�ڵ�
        }

        //�����Ҷ�ڵ㣬Node Expansion����չ��һ��ڵ�
        if (childrenCountMax > childrenCount){
            treeNode* newNode = new treeNode(board, childrenAction[childrenCount], this, countPointer); //��չһ���ӽڵ�
            children[childrenCount] = newNode;
            childrenCount++; //����չ���ӽڵ���++
            return newNode;
        }

        //���㵱ǰ�ڵ��ÿ���ӽڵ��UCBֵ������ĳ���ڵ㣩
        for (int i = 0; i < childrenCount; ++i){
            children[i]->UCB = children[i]->value / double(children[i]->n) + 0.272 * sqrt(log(double(*countPointer)) / double(children[i]->n)); //UCB��ʽ
        }
        int bestChild = 0;
        double maxUCB = 0;

        //�ҳ������ӽڵ���UCBֵ�����ӽڵ�
        for (int i = 0; i < childrenCount; ++i){
            if (maxUCB < children[i]->UCB){
                bestChild = i;
                maxUCB = children[i]->UCB;
            }
        }
        return children[bestChild]->treeRules(); //��UCB�����ӽڵ������һ������
    }
    //ģ��
    double simulation(){
        int simulationBoard[9][9];
        int board_opp[9][9]; //��������
        int res[9][9];
        double x = 0, y = 0;
        vector<int> available_list; //�Ϸ�λ�ñ�
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
            x += getValidPositions(simulationBoard, res);     //���ӷ�����λ����
            y += getValidPositions(board_opp, res); //�����ӷ�����λ����
        }
            return (double)(x - y)/10;
    }
    void backup(double deltaValue){ //�ش���ֵ,�ӵ�ǰҶ�ڵ��Լ����ϵ�ÿһ�����ڵ㶼���Ϲ�ֵ
        treeNode* node = this;
        int side = 0;
        while (node != nullptr){ //��node���Ǹ��ڵ�ĸ��ڵ�ʱ
            if (side == 1){ //���ӷ�
                node->value += deltaValue;
                side--;
            }
            else{ //�����ӷ�
                node->value -= deltaValue;
                side++;
            }
            node->n++; //��ǰ�ڵ㱻̽������++
            node = node->parent;
        }
    }

private:
    void evaluate(){ //�������µ�λ��,�޸���childrenCountMax��childrenAction
        int result[9][9];
        int validPositionCount = getValidPositions(board, result); //���µ�λ����
        int validPositions[MAXBranchNum];                          //���µ�λ������
        int availableNum = 0;
        for (int i = 0; i < 9; ++i){
            for (int j = 0; j < 9; ++j){
                if (result[i][j]){
                    validPositions[availableNum] = i * 9 + j; //���µ�λ��
                    availableNum++;                           //���µ�λ����
                }
            }
        }
        childrenCountMax = validPositionCount; //�ܹ����µ�λ����
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
    //loadimage(&abyssmagecover, "res\\��Ԩ��ʦcover.jpg", 400, 400, true);
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
    settextstyle(40, 0, "����");
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
    int flag = 0;//��ֹ�ĵ㶼����
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
    for (int i = 0; i < 4; i++){ //�ж������ⲽ��Χλ�õ������Ƿ�����
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)){ //��������
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) //���������ӵ�λ�ã���Ƿ��ʹ�������ѭ����
                if (!air_judge(board, x_dx, y_dy)) {                            //�������(x_dx,y_dy)û���ˣ�������
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
    int count = 0; //�ܼ���Ľڵ�������̽��������UCB�е�N��
    srand(clock());
    int start = clock();
    int timeout = (int)(0.98 * (double)CLOCKS_PER_SEC);
    int opp_action[2] = { LastChess.X, LastChess.Y }; //������һ����������
    treeNode rootNode(board, opp_action, nullptr, &count); //�������ڵ㣬���ڵ�ĸ��ڵ�Ϊ��
    while (clock() - start < timeout){
        count++;                                //����Ľڵ���++
        treeNode* node = rootNode.treeRules(); //��չһ�Σ�nodeָ�����һ����չ��Ҷ�ڵ�
        double result = node->simulation();     //�����ֵ
        node->backup(result);
    }
    int bestChildren[MAXBranchNum] = { 0 }; //���������ӽڵ�����
    int bestChildrenNum = 0;              //�����ӽڵ����
    int maxValue = INT_MIN;  //��ǰ�����ӽڵ����
    for (int i = 0; i < rootNode.childrenCount; ++i){
        if (maxValue < rootNode.children[i]->value){
            //����
            memset(bestChildren, 0, sizeof(bestChildren));
            bestChildrenNum = 0;

            bestChildren[bestChildrenNum++] = i;
            maxValue = rootNode.children[i]->value;
        }
        else if (maxValue == rootNode.children[i]->value){
            bestChildren[bestChildrenNum++] = i;
        }
    }
    int random = rand() % bestChildrenNum;                         //��������������ѡһ��
    int* bestAction = rootNode.childrenAction[bestChildren[random]]; //�����ӽڵ��Ӧ�߷�
    AIX = bestAction[0];
    AIY = bestAction[1];
}

void AIHelp(){
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            board[i][j] = -board[i][j];
    //����Ƿ��վ�
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
    settextstyle(80, 0, "����");
    outtextxy(210, 310, "ϵͳ����");
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
    outtextxy(860, 730, "���沢�˳�");
    settextcolor(RGB(0, 0, 0));
    outtextxy(860, 655, "����");
    settextcolor(RGB(0, 0, 0));
    outtextxy(1020, 655, "����");
    settextcolor(RGB(0, 0, 0));
    outtextxy(860, 580, "��ʾ");
    settextcolor(RGB(0, 0, 0));
    outtextxy(1020, 580, "����");
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
            outtextxy(860, 730, "���沢�˳�");   //�������
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(860, 730, "���沢�˳�");
        }
        //renshu
        if (mouseX >= 850 && mouseX <= 990 && mouseY >= 650 && mouseY <= 700){
            settextcolor(RGB(255, 0, 0));
            outtextxy(860, 655, "����");   //�������
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(860, 655, "����");
        }
        //bgm
        if (mouseX >= 1020 && mouseX <= 1150 && mouseY >= 650 && mouseY <= 700){
            settextcolor(RGB(255, 0, 0));
            outtextxy(1020, 655, "����");   //�������
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(1020, 655, "����");
        }
        //��ʾ
        if (mouseX >= 850 && mouseX <= 990 && mouseY >= 575 && mouseY <= 625){
            settextcolor(RGB(255, 0, 0));
            outtextxy(860, 580, "��ʾ");   //�������
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(860, 580, "��ʾ");
        }
        //����
        if (mouseX >= 1010 && mouseX <= 1150 && mouseY >= 575 && mouseY <= 625){
            settextcolor(RGB(255, 0, 0));
            outtextxy(1020, 580, "����");   //�������
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(1020, 580, "����");
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
                        outtextxy(860, 655, "�¶Ծ�");   //�������
                    }
                    else {
                        settextcolor(RGB(0, 0, 0));
                        outtextxy(860, 655, "�¶Ծ�");
                    }
                    //quit
                    if (mouseX >= 850 && mouseX <= 1150 && mouseY >= 725 && mouseY <= 775) {
                        settextcolor(RGB(255, 0, 0));
                        outtextxy(860, 730, "�˳�");   //�������
                    }
                    else {
                        settextcolor(RGB(0, 0, 0));
                        outtextxy(860, 730, "�˳�");
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
            //����
            if (mouseX >= 1020 && mouseX <= 1150 && mouseY >= 650 && mouseY <= 700) {
                mciSendString(_T(music[musicline][1]), NULL, 0, NULL);
                musicline++;
                if (musicline == 11)musicline = 0;
                mciSendString(music[musicline][0], NULL, 0, NULL);
            }
            //��ʾ
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
                    outtextxy(860, 580, "Ͷ�˰�");
                Sleep(2000);
                setfillcolor(RGB(225, 133, 72));
                fillrectangle(850, 575, 990, 625);
            }
            //����
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
                //��ԭ���غ�ǰ���
            }
            //����
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
                //������
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

                        //����
                        if (mouseX >= 1010 && mouseX <= 1150 && mouseY >= 575 && mouseY <= 625) {
                            settextcolor(RGB(255, 0, 0));
                            outtextxy(1020, 580, "����");   //�������
                        }
                        else {
                            settextcolor(RGB(0, 0, 0));
                            outtextxy(1020, 580, "����");
                        }

                        //�ٰ�һ��
                        if (mouseX >= 850 && mouseX <= 990 && mouseY >= 650 && mouseY <= 700) {
                            settextcolor(RGB(255, 0, 0));
                            outtextxy(860, 655, "�¶Ծ�");   //�������
                        }
                        else {
                            settextcolor(RGB(0, 0, 0));
                            outtextxy(860, 655, "�¶Ծ�");
                        }

                        //quit
                        if (mouseX >= 850 && mouseX <= 1020 && mouseY >= 725 && mouseY <= 775){
                            settextcolor(RGB(255, 0, 0)); 
                            outtextxy(860, 730, "�˳�");   //�������
                        }
                        else{
                            settextcolor(RGB(0, 0, 0));
                            outtextxy(860, 730, "�˳�");
                        }
                        if (msg.message == WM_LBUTTONDOWN){
                            //����
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
                //����Ƿ������µĵط�
                for (int i = 0; i < 9; i++){
                    for (int j = 0; j < 9; j++)
                        if (put_available(board, i, j, -1)){
                            AILose = 0;
                            break;
                        }
                    if (!AILose)
                        break;
                }
                //���������
                if (!AILose){
                    AIGo();
                    chessDown(AIX, AIY, -myColor);
                    firstTurn = false;
                }
                //������һ�ʤ
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
    settextstyle(45, 0, "����");    //��������ķ��
    while (1){
        ExMessage msg = getmessage(EX_MOUSE);
        mouseX = msg.x;
        mouseY = msg.y;
        //quit
        if (mouseX >= 600 && mouseX <= 800 && mouseY >= 700 && mouseY <= 750){
            settextcolor(RGB(255, 0, 0));
            outtextxy(650,705 , "�˳�");   //�������
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(650, 705, "�˳�");
        }
        //continue
        if (mouseX >= 350 && mouseX <= 550 && mouseY >= 700 && mouseY <= 750){
            settextcolor(RGB(255, 0, 0));
            outtextxy(360, 705, "������Ϸ");   //�������
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(360, 705, "������Ϸ");
        }
        //New Game
        if (mouseX >= 100 && mouseX <= 300 && mouseY >= 700 && mouseY <= 750){
            settextcolor(RGB(255, 0, 0));
            outtextxy(130, 705, "����Ϸ");   //�������
        }
        else{
            settextcolor(RGB(0, 0, 0));
            outtextxy(130, 705, "����Ϸ");
        }
        //����¼�
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
                    //ִ��
                    if (mouseX >= 100 && mouseX <= 190 && mouseY >= 700 && mouseY <= 750){
                        settextcolor(RGB(255, 0, 0));
                        outtextxy(100, 705, "ִ��");   //�������
                    }
                    else{
                        settextcolor(RGB(0, 0, 0));
                        outtextxy(100, 705, "ִ��");
                    }
                    //ִ��
                    if (mouseX >= 210 && mouseX <= 300 && mouseY >= 700 && mouseY <= 750){
                        settextcolor(RGB(255, 0, 0));
                        outtextxy(210, 705, "ִ��");   //�������
                    }
                    else{
                        settextcolor(RGB(0, 0, 0));
                        outtextxy(210, 705, "ִ��");
                    }
                    if (msg.message == WM_LBUTTONDOWN){
                        if (mouseX >= 100 && mouseX <= 190 && mouseY >= 700 && mouseY <= 750){//ִ��
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